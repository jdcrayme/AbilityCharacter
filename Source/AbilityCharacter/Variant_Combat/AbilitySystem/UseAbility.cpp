// Copyright Epic Games, Inc. All Rights Reserved.

#include "UseAbility.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/CharacterAttributeSet.h"
#include "Blueprint/UserWidget.h"
#include "CombatGameplayTags.h"
#include "CombatInputLabelProvider.h"
#include "CombatUsePromptActor.h"
#include "CombatUsePromptInterface.h"
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

        // the built-in prompt works out of the box; assign any widget class
        // implementing the CombatUsePromptInterface in a Blueprint subclass to restyle it
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

        CurrentUsable = TScriptInterface<ICombatUsable>();
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

TScriptInterface<ICombatUsable> UUseAbility::ResolveUsable(const FHitResult& Hit) const
{
        AActor* HitActor = Hit.GetActor();
        if (!HitActor)
        {
                return TScriptInterface<ICombatUsable>();
        }

        // actors implementing the interface directly take priority
        if (HitActor->Implements<UCombatUsable>())
        {
                return TScriptInterface<ICombatUsable>(HitActor);
        }

        // otherwise any of the actor's components can provide the interaction
        // (action volume components on vehicles, doors, NPCs...)
        TInlineComponentArray<UActorComponent*> Components(HitActor);
        for (UActorComponent* Component : Components)
        {
                if (Component && Component->Implements<UCombatUsable>())
                {
                        return TScriptInterface<ICombatUsable>(Component);
                }
        }

        return TScriptInterface<ICombatUsable>();
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
                        if (CurrentUsable.GetObject())
                        {
                                CurrentUsable = TScriptInterface<ICombatUsable>();
                                HidePrompt();
                        }
                        return;
                }
        }

        // linear ray ahead of the character, following the view. Prefer the player
        // camera over the avatar's eyes so the trace keeps following the view while
        // the player controls something else (e.g. a possessed vehicle).
        FVector TraceStart;
        FRotator ViewRotation;
        const APawn* Pawn = Cast<APawn>(Avatar);
        const APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
        if (PC)
        {
                PC->GetPlayerViewPoint(TraceStart, ViewRotation);
        }
        else
        {
                Avatar->GetActorEyesViewPoint(TraceStart, ViewRotation);
        }
        const FVector TraceEnd = TraceStart + ViewRotation.Vector() * TraceDistance;

        FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(UseAbilityTrace), false);
        QueryParams.AddIgnoredActor(Avatar);
        if (PromptActor)
        {
                QueryParams.AddIgnoredActor(PromptActor);
        }

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

        // only blocking hits against usable targets count; unusable ones are skipped
        TScriptInterface<ICombatUsable> Usable;
        if (bBlockingHit)
        {
                Usable = ResolveUsable(Hit);
                if (Usable.GetObject() && !ICombatUsable::Execute_CanUse(Usable.GetObject(), Avatar))
                {
                        Usable = TScriptInterface<ICombatUsable>();
                }
        }

        const bool bTargetChanged = (Usable.GetObject() != CurrentUsable.GetObject());
        CurrentUsable = Usable;

        if (bTargetChanged)
        {
                if (CurrentUsable.GetObject())
                {
                        UE_LOG(LogTemp, Verbose, TEXT("[UseAbility] found usable target %s"), *GetNameSafe(CurrentUsable.GetObject()));
                }
                else
                {
                        HidePrompt();
                }
        }

        if (CurrentUsable.GetObject())
        {
                // refresh the prompt on every check so its text and position stay in sync
                UpdatePrompt(TraceStart, Hit.ImpactPoint, bTargetChanged);
        }
}

void UUseAbility::OnUsePressed(FGameplayEventData EventData)
{
        // the use input only matters on the machine that is running the trace
        if (!IsLocalPlayerAvatar())
        {
                return;
        }

        // diagnostic checkpoint: proves the gameplay event reached this ability
        UE_LOG(LogTemp, Log, TEXT("[UseAbility] use input received"));

        // refresh the interaction ray so the target can't be used after stepping out of range
        RunUseCheck();

        AActor* Avatar = GetAvatarActorFromActorInfo();
        UObject* UsableObject = CurrentUsable.GetObject();
        if (!Avatar || !UsableObject)
        {
                // diagnostic checkpoint: the input chain works, but the ray has no usable target right now
                UE_LOG(LogTemp, Warning, TEXT("[UseAbility] use input received, but no usable target is in range"));
                return;
        }

        // usable targets can become unusable between traces (single use, state changes)
        if (!ICombatUsable::Execute_CanUse(UsableObject, Avatar))
        {
                CurrentUsable = TScriptInterface<ICombatUsable>();
                HidePrompt();
                return;
        }

        UE_LOG(LogTemp, Warning, TEXT("[UseAbility] %s used %s"), *Avatar->GetName(), *GetNameSafe(UsableObject));

        // call the function on the action volume actor or component
        ICombatUsable::Execute_Use(UsableObject, Avatar);

        // single-use volumes drop out of focus right away
        if (!ICombatUsable::Execute_CanUse(UsableObject, Avatar))
        {
                CurrentUsable = TScriptInterface<ICombatUsable>();
                HidePrompt();
        }
}

void UUseAbility::OnAvatarDestroyed(AActor* DestroyedActor)
{
        // the ability dies with its avatar: release the prompt so it doesn't linger in the world
        CurrentUsable = TScriptInterface<ICombatUsable>();
        DestroyPrompt();
}

void UUseAbility::UpdatePrompt(const FVector& ViewLocation, const FVector& HitLocation, bool bSnapToTarget)
{
        if (!PromptWidgetClass || !CurrentUsable.GetObject() || !GetWorld())
        {
                return;
        }

        // spawn the world-space prompt holder once; the ability owns its whole lifecycle
        if (!PromptActor)
        {
                PromptActor = GetWorld()->SpawnActorDeferred<ACombatUsePromptActor>(
                        ACombatUsePromptActor::StaticClass(),
                        FTransform::Identity,
                        nullptr,
                        nullptr,
                        ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

                if (PromptActor)
                {
                        PromptActor->SetupPrompt(PromptWidgetClass, PromptSpace, PromptDrawSize, PromptSmoothingSpeed);
                        PromptActor->FinishSpawning(FTransform::Identity);
                }
        }

        if (!PromptActor)
        {
                return;
        }

        // sit along the interaction ray, by default midway between the ray source
        // and the intersection point, facing the player
        const FVector PromptLocation = ViewLocation + (HitLocation - ViewLocation) * PromptDistanceRatio;
        PromptActor->SetPromptTarget(PromptLocation, ViewLocation, bSnapToTarget || !bPromptVisible);

        // the key label comes from the avatar's input configuration when it
        // provides one (CharacterBase reads it from its EventInputMappings);
        // avatars without the CombatInputLabelProvider interface fall back to
        // the ability's own UseKeyLabel
        FText KeyText = UseKeyLabel;
        if (AActor* AvatarActor = GetAvatarActorFromActorInfo())
        {
                if (AvatarActor->Implements<UCombatInputLabelProvider>())
                {
                        const FText AvatarKeyLabel = ICombatInputLabelProvider::Execute_GetKeyLabelForEvent(AvatarActor, TAG_Event_InputUsePressed);
                        if (!AvatarKeyLabel.IsEmpty())
                        {
                                KeyText = AvatarKeyLabel;
                        }
                }
        }

        // push the key label and the target's action text through the prompt interface;
        // any widget implementing CombatUsePromptInterface can receive them
        if (UUserWidget* PromptWidget = PromptActor->GetPromptWidget())
        {
                if (PromptWidget->Implements<UCombatUsePromptInterface>())
                {
                        ICombatUsePromptInterface::Execute_UpdatePromptText(
                                PromptWidget,
                                KeyText,
                                ICombatUsable::Execute_GetUseText(CurrentUsable.GetObject(), GetAvatarActorFromActorInfo()));
                }
        }

        PromptActor->SetActorHiddenInGame(false);
        bPromptVisible = true;
}

void UUseAbility::HidePrompt()
{
        if (PromptActor)
        {
                PromptActor->SetActorHiddenInGame(true);
        }
        bPromptVisible = false;
}

void UUseAbility::DestroyPrompt()
{
        if (PromptActor)
        {
                PromptActor->Destroy();
                PromptActor = nullptr;
        }
        bPromptVisible = false;
}
