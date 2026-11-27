// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CombatAbilityBase.generated.h"

class UAnimMontage;
class UGameplayEffect;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
struct FGameplayEventData;

/**
 *  Base class for all combat abilities.
 *  Runs the full ability -> effect -> cue chain:
 *  1. Plays its montage through an ability task.
 *  2. Warns nearby damageable actors of the incoming attack.
 *  3. Listens for Combat.AttackTrace gameplay events sent by the montage's
 *     AnimNotify_DoAttackTrace to run the hit sweep.
 *  4. Applies the damage effect attacker-side with hit result and knockback in
 *     the effect context, then executes the GameplayCue.Damage cue on the
 *     victim so it can react.
 *
 *  Melee and ranged weapons derive from this class through UMeleeAbility / URangedAbility.
 */
UCLASS(Abstract)
class ABILITYCHARACTER_API UCombatAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()

public:

	UCombatAbilityBase();

	/** Montage to play when the ability activates */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Animation")
	TObjectPtr<UAnimMontage> MontageToPlay;

	/** Gameplay effect applied to each target hit by this attack */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Effects")
	TSubclassOf<UGameplayEffect> EffectToApply;

	/** Damage passed to the effect through the Data.Damage SetByCaller magnitude */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Damage", meta = (ClampMin = 0, ClampMax = 100))
	float AttackDamage = 10.0f;

	/** Amount of knockback impulse a hit applies to the victim */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Damage", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm/s"))
	float KnockbackImpulse = 250.0f;

	/** Amount of upwards impulse a hit applies to the victim */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Damage", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm/s"))
	float LaunchImpulse = 300.0f;

	/** Distance ahead of the avatar that melee attack sweeps will extend */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Targeting", meta = (ClampMin = 0, ClampMax = 500, Units = "cm"))
	float MeleeTraceDistance = 75.0f;

	/** Radius of the sphere sweep for melee attacks */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Targeting", meta = (ClampMin = 0, ClampMax = 200, Units = "cm"))
	float MeleeTraceRadius = 50.0f;

	/** Distance ahead of the avatar that damageable actors will be warned of the incoming attack */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Targeting", meta = (ClampMin = 0, ClampMax = 500, Units = "cm"))
	float DangerTraceDistance = 200.0f;

	/** Radius of the sphere sweep used to warn damageable actors of the incoming attack */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Targeting", meta = (ClampMin = 0, ClampMax = 200, Units = "cm"))
	float DangerTraceRadius = 20.0f;

	/** If true, attack sweeps also hit world-dynamic objects such as destructibles. AI attacks usually only hit pawns */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Targeting")
	bool bDamageWorldObjects = true;

	/** Stamina drained when this ability commits. Applied through the shared stamina cost effect */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Cost", meta = (ClampMin = 0, ClampMax = 100))
	float StaminaCost = 10.0f;

protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	/** Warns nearby damageable actors that an attack is coming so they can react */
	void NotifyEnemiesOfIncomingAttack();

	/** Runs the hit sweep described by the attack trace event and damages each target */
	void PerformHitCheck(const FGameplayEventData& EventData);

	/** Applies the damage effect and damage cue to a single target */
	void ApplyDamageToTarget(const FHitResult& HitResult);

	/** Called when the montage finishes, is interrupted or cancelled */
	UFUNCTION()
	void OnMontageFinished();

	/** Called when a montage AnimNotify fires the attack trace event */
	UFUNCTION()
	virtual void OnAttackTraceEvent(FGameplayEventData EventData);

	/** Task playing the attack montage */
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	/** Task listening for attack trace events from the montage notifies */
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> TraceEventTask;
};
