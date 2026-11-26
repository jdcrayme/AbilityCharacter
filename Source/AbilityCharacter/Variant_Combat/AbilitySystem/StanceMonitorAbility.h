// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "StanceMonitorAbility.generated.h"

/**
 *  Passive ability that drives the character's stance from the game context.
 *  Runs a periodic sweep for enemies within detection range (and listens for
 *  arena-entered gameplay events from CombatActivationVolume):
 *  - When combat is detected, the combat stance ability is activated, which
 *    grants the attack abilities and sets the State.Stance.Combat tag.
 *  - After no enemies have been seen for the exit delay, the neutral stance
 *    ability is activated, which revokes the attacks again.
 *
 *  Additional stance triggers (sneaking zones, prone areas...) can be added by
 *  sending more context events and extending the monitor.
 */
UCLASS()
class ABILITYCHARACTER_API UStanceMonitorAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:

	UStanceMonitorAbility();

	/** Radius in which nearby enemies put the character into the combat stance */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Stance Monitor", meta = (ClampMin = 0, Units = "cm"))
	float CombatDetectRadius = 600.0f;

	/** How long the character stays in the combat stance after the last enemy was seen */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Stance Monitor", meta = (ClampMin = 0, Units = "s"))
	float CombatExitDelay = 5.0f;

	/** How often the enemy sweep runs */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Stance Monitor", meta = (ClampMin = 0.1, Units = "s"))
	float SweepInterval = 0.5f;

	/** Actor tag that marks hostiles for the sweep (enemies for players, the player for AI) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Stance Monitor")
	FName HostileActorTag = FName("Enemy");

protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/** Passive ability: start running as soon as it is granted */
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	/** Sweeps for nearby enemies and activates the matching stance */
	void RunStanceCheck();

	/** Puts the character into the combat stance */
	void EnterCombatStance();

	/** Puts the character back into the neutral stance */
	void EnterNeutralStance();

private:

	/** Called by the periodic sweep timer */
	UFUNCTION()
	void OnSweepTimer();

	/** Called when a combat arena entry event arrives */
	UFUNCTION()
	void OnArenaEntered(FGameplayEventData EventData);

	/** Game time the last enemy was seen */
	float LastEnemySeenTime = -1000.0f;

	/** Whether the character is currently in the combat stance */
	bool bInCombatStance = false;

	/** Periodic sweep timer */
	FTimerHandle SweepTimerHandle;

	UPROPERTY()
	TObjectPtr<class UAbilityTask_WaitGameplayEvent> ArenaEventTask;
};
