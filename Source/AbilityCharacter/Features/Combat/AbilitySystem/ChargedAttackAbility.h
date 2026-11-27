// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Features/Combat/AbilitySystem/MeleeAbility.h"
#include "ChargedAttackAbility.generated.h"

class UAbilityTask_WaitGameplayEvent;
struct FGameplayEventData;

/**
 *  A melee attack that charges while the input is held.
 *  The montage's AnimNotify_CheckChargedAttack fires the Combat.CheckChargedAttack
 *  event; the ability jumps between the charge loop and attack sections based on
 *  whether the input is still held (players) or on a randomized loop count (AI,
 *  see UAIChargedAttackAbility).
 */
UCLASS()
class ABILITYCHARACTER_API UChargedAttackAbility : public UMeleeAbility
{
	GENERATED_BODY()

public:

	UChargedAttackAbility();

	/** Name of the montage section that loops while the attack is charging */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Charged")
	FName ChargeLoopSection;

	/** Name of the montage section that plays the charged attack */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Charged")
	FName ChargeAttackSection;

protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** Whether the charge should be released into the attack instead of looping again */
	virtual bool ShouldReleaseCharge();

	/** Called when the montage notify asks whether to keep charging */
	UFUNCTION()
	virtual void OnChargeCheckEvent(FGameplayEventData EventData);

	/** Called when the charge input is released */
	UFUNCTION()
	virtual void OnChargeReleasedEvent(FGameplayEventData EventData);

	/** Whether the charge loop has run at least once */
	bool bHasLoopedCharge = false;

	/** Whether the charge input has been released */
	bool bChargeReleased = false;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> ChargeCheckTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> ChargeReleaseTask;
};
