// Copyright Epic Games, Inc. All Rights Reserved.

#include "StanceMonitorAbility.h"
#include "AbilityCharacter.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "Core/AbilitySystem/CombatGameplayTags.h"
#include "Engine/OverlapResult.h"
#include "TimerManager.h"

void UStanceMonitorAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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

	// arena volumes tell us when the character walks into a fight
	ArenaEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TAG_Event_ArenaEntered, nullptr, false, false);
	ArenaEventTask->EventReceived.AddDynamic(this, &UStanceMonitorAbility::OnArenaEntered);
	ArenaEventTask->ReadyForActivation();

	// periodic enemy sweep
	LastEnemySeenTime = GetWorld()->GetTimeSeconds();
	GetWorld()->GetTimerManager().SetTimer(SweepTimerHandle, this, &UStanceMonitorAbility::OnSweepTimer, SweepInterval, true);
}

void UStanceMonitorAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SweepTimerHandle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UStanceMonitorAbility::OnSweepTimer()
{
	UE_LOG(LogAbilityCharacter, Verbose, TEXT("[StanceMonitor] sweep tick"));
	RunStanceCheck();
}

void UStanceMonitorAbility::OnArenaEntered(FGameplayEventData EventData)
{
	// walking into an arena immediately puts us on guard
	UE_LOG(LogAbilityCharacter, Warning, TEXT("[StanceMonitor] arena entered -> combat stance"));
	LastEnemySeenTime = GetWorld()->GetTimeSeconds();
	EnterCombatStance();
}

void UStanceMonitorAbility::RunStanceCheck()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	UWorld* World = GetWorld();
	if (!Avatar || !World) return;

	const float Now = World->GetTimeSeconds();

	// sweep for enemy pawns around us
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Avatar);

	const bool bOverlap = World->OverlapMultiByObjectType(
		Overlaps,
		Avatar->GetActorLocation(),
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(CombatDetectRadius),
		QueryParams);

	bool bEnemyNearby = false;
	if (bOverlap)
	{
		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* OtherActor = Overlap.GetActor();
			if (OtherActor && OtherActor != Avatar && OtherActor->ActorHasTag(HostileActorTag))
			{
				bEnemyNearby = true;
				break;
			}
		}
	}

	if (bEnemyNearby)
	{
		LastEnemySeenTime = Now;

		if (!bInCombatStance)
		{
			EnterCombatStance();
		}
		return;
	}

	// no enemies for a while: stand down
	if (bInCombatStance && (Now - LastEnemySeenTime) >= CombatExitDelay)
	{
		UE_LOG(LogAbilityCharacter, Warning, TEXT("[StanceMonitor] no enemies for %.1fs -> neutral stance"), Now - LastEnemySeenTime);
		EnterNeutralStance();
	}
}

void UStanceMonitorAbility::EnterCombatStance()
{
	bInCombatStance = true;

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(TAG_Ability_Stance_Combat));
	}
}

void UStanceMonitorAbility::EnterNeutralStance()
{
	bInCombatStance = false;

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(TAG_Ability_Stance_Neutral));
	}
}
