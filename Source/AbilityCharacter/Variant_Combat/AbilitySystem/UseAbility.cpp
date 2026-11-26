// Copyright Epic Games, Inc. All Rights Reserved.

#include "UseAbility.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/CharacterAttributeSet.h"
#include "Blueprint/UserWidget.h"
#include "CombatGameplayTags.h"
#include "CombatUsable.h"
#include "CombatUsePromptWidget.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

UUseAbility::UUseAbility()
{
	// passive: granted with the default abilities and runs for the character's lifetime
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bRetriggerInstancedAbility = false;

	// runs on the owning client as well so the trace and prompt follow the local camera
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// the prompt works out of the box; assign a Blueprint subclass to restyle it
	PromptWidgetClass = UCombatUsePromptWidget::StaticClass();
	UseKeyLabel = NSLOCTEXT("UseAbility", "DefaultUseKeyLabel", "E");
}

void UUseAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	// start the ability as soon as it is granted; granting alone doesn't activate
	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid() &&
		ActorInfo->OwnerActor.IsValid() &&
		ActorInfo->OwnerActor->GetLocalRole() == ROLE_Authority &&
		!Spec.IsActive())
	{
		ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle, false);
	}
}

void UUseAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// the use input arrives as a gameplay event, mirroring the attack input events
	UseEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TAG_Event_InputUsePressed);
	UseEventTask->EventReceived.AddDynamic(this, &UUseAbility::OnUsePressed);
	UseEventTask->ReadyForActivation();

	// clean the prompt up when the avatar (e.g. on respawn) is destroyed
	Avatar->OnDestroyed.AddUniqueDynamic(this, &UUseAbility::OnAvatarDestroyed);

	// periodic interaction trace
	GetWorld()->GetTimerManager().SetTimer(TraceTimerHandle, this, &UUseAbility::OnTraceTimer, TraceInterval, true);
}

void UUseAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		Avatar->OnDestroyed.RemoveDynamic(this, &UUseAbility::OnAvatarDestroyed);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TraceTimerHandle);
	}

	CurrentUsableActor = nullptr;
	DestroyPrompt();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UUseAbility::IsLocalPlayerAvatar() const
{
	// only the local player's machine drives the trace and the prompt; AI ability
	// instances and server instances for remote players stay idle
	const APawn* Pawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	return Pawn && Pawn->IsPlayerControlled() && Pawn->IsLocallyControlled();
}

void UUseAbility::OnTraceTimer()
{
	RunUseCheck();
}

void UUseAbility::RunUseCheck()
{
	if (!IsLocalPlayerAvatar())
	{
		return;
	}

	AActor* Avatar = GetAvatarActorFromActorInfo();
	UWorld* World = GetWorld();
	if (!Avatar || !World)
	{
		return;
	}

	// dead characters don't interact
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		if (ASC->HasMatchingGameplayTag(Tag_State_Dead))
		{
			if (CurrentUsableActor)
			{
				CurrentUsableActor = nullptr;
				HidePrompt();
			}
			return;
		}
	}

	// linear ray ahead of the character, following the view direction
	FVector TraceStart;
	FRotator ViewRotation;
	Avatar->GetActorEyesViewPoint(TraceStart, ViewRotation);
	const FVector TraceEnd = TraceStart + ViewRotation.Vector() * TraceDistance;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(UseAbilityTrace), false);
	QueryParams.AddIgnoredActor(Avatar);

	FHitResult Hit;
	const bool bBlockingHit = World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, TraceChannel, QueryParams);

	if (bDebugTrace)
	{
		DrawDebugLine(World, TraceStart, TraceEnd, bBlockingHit ? FColor::Green : FColor::Silver, false, TraceInterval, 0, 1.0f);
		if (bBlockingHit)
		{
			DrawDebugPoint(World, Hit.ImpactPoint, 8.0f, FColor::Green, false, TraceInterval);
		}
	}

	// only blocking hits against usable actors count; the first one wins
	AActor* UsableActor = nullptr;
	if (bBlockingHit && Hit.GetActor() && Hit.GetActor()->Implements<UCombatUsable>())
	{
		AActor* Candidate = Hit.GetActor();
		if (ICombatUsable::Execute_CanUse(Candidate, Avatar))
		{
			UsableActor = Candidate;
		}
	}

	if (UsableActor != CurrentUsableActor)
	{
		CurrentUsableActor = UsableActor;

		if (CurrentUsableActor)
		{
			UE_LOG(LogTemp, Verbose, TEXT("[UseAbility] found usable actor %s"), *CurrentUsableActor->GetName());
			ShowPrompt(CurrentUsableActor);
		}
		else
		{
			HidePrompt();
		}
	}
	else if (CurrentUsableActor)
	{
		// keep the prompt in sync: the volume may change its use text at any time
		ShowPrompt(CurrentUsableActor);
	}
}

void UUseAbility::OnUsePressed(FGameplayEventData EventData)
{
	// the use input only matters on the machine that is running the trace
	if (!IsLocalPlayerAvatar())
	{
		return;
	}

	// refresh the interaction ray so the volume can't be used after stepping out of range
	RunUseCheck();

	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar || !CurrentUsableActor)
	{
		return;
	}

	// usable actors can become unusable between traces (single use, state changes)
	if (!ICombatUsable::Execute_CanUse(CurrentUsableActor, Avatar))
	{
		CurrentUsableActor = nullptr;
		HidePrompt();
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[UseAbility] %s used %s"), *Avatar->GetName(), *CurrentUsableActor->GetName());

	// call the function on the action volume
	ICombatUsable::Execute_Use(CurrentUsableActor, Avatar);

	// single-use volumes drop out of focus right away
	if (!ICombatUsable::Execute_CanUse(CurrentUsableActor, Avatar))
	{
		CurrentUsableActor = nullptr;
		HidePrompt();
	}
}

void UUseAbility::OnAvatarDestroyed(AActor* DestroyedActor)
{
	// the ability dies with its avatar: release the prompt so it doesn't linger on the viewport
	CurrentUsableActor = nullptr;
	DestroyPrompt();
}

void UUseAbility::ShowPrompt(AActor* UsableActor)
{
	if (!UsableActor || !PromptWidgetClass)
	{
		return;
	}

	// the widget lives on the local player's screen
	APlayerController* PC = nullptr;
	if (const APawn* Pawn = Cast<APawn>(GetAvatarActorFromActorInfo()))
	{
		PC = Cast<APlayerController>(Pawn->GetController());
	}

	if (!PromptWidget && PC)
	{
		PromptWidget = CreateWidget<UCombatUsePromptWidget>(PC, PromptWidgetClass);
		if (PromptWidget)
		{
			PromptWidget->AddToViewport(0);

			// anchor the prompt near the bottom center of the screen
			PromptWidget->SetAnchorsInViewport(FAnchors(0.5f, 0.875f));
			PromptWidget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
			PromptWidget->SetPositionInViewport(FVector2D::ZeroVector, false);
		}
	}

	if (PromptWidget)
	{
		PromptWidget->SetPromptText(UseKeyLabel, ICombatUsable::Execute_GetUseText(UsableActor, GetAvatarActorFromActorInfo()));
		PromptWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		bPromptVisible = true;
	}
}

void UUseAbility::HidePrompt()
{
	if (PromptWidget)
	{
		PromptWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	bPromptVisible = false;
}

void UUseAbility::DestroyPrompt()
{
	if (PromptWidget)
	{
		PromptWidget->RemoveFromParent();
		PromptWidget = nullptr;
	}
	bPromptVisible = false;
}
