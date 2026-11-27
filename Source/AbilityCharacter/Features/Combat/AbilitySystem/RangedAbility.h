// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Features/Combat/AbilitySystem/CombatAbilityBase.h"
#include "RangedAbility.generated.h"

/**
 *  Base class for ranged attack abilities (a pistol, for example).
 *  Follows the same montage -> trace event -> effect -> cue chain as melee
 *  attacks: the hitscan runs when the montage hits its attack trace notify
 *  (AnimNotify_DoAttackTrace, or the generic AnimNotify_SendGameplayEvent with
 *  the Event.AttackTrace tag), so the shot lands on the exact frame the
 *  animation fires. Montages without a trace notify still fire once when the
 *  montage finishes.
 *
 *  The trace originates from the controlling player's view when there is one
 *  (so the shot goes where the crosshair points) and from the muzzle socket
 *  otherwise (AI and anything else that aims through its body).
 */
UCLASS()
class ABILITYCHARACTER_API URangedAbility : public UCombatAbilityBase
{
	GENERATED_BODY()

public:

	URangedAbility();

	/** Socket on the weapon or character mesh that shots originate from */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Ranged")
	FName MuzzleSocketName;

	/** Maximum distance of the hitscan trace */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Ranged", meta = (ClampMin = 0, Units = "cm"))
	float TraceDistance = 5000.0f;

protected:

	/** Resets the fire flag, then commits and starts the montage through the base chain */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** The montage's trace notify is the fire frame: run the hitscan */
	virtual void OnAttackTraceEvent(FGameplayEventData EventData) override;

	/** Fallback for montages without a trace notify: fire once at the end */
	virtual void OnMontageFinished() override;

	/** Runs the hitscan from the view or muzzle and applies the damage chain to whatever it hits */
	void PerformRangedTrace();

protected:

	/** Whether the trace notify (or the end-of-montage fallback) already fired this activation */
	bool bHasFired = false;
};
