// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MeleeAbility.h"
#include "ComboAttackAbility.generated.h"

class UAbilityTask_WaitGameplayEvent;
struct FGameplayEventData;

/**
 *  A melee attack that plays a combo string.
 *  The montage's AnimNotify_CheckCombo fires the Combat.CheckCombo event; the
 *  ability then decides whether to jump to the next combo section.
 *  Player characters buffer attack input through Combat.Input.AttackPressed
 *  events, AI characters decide the combo length up front (see UAIComboAttackAbility).
 */
UCLASS()
class ABILITYCHARACTER_API UComboAttackAbility : public UMeleeAbility
{
	GENERATED_BODY()

public:

	UComboAttackAbility();

	/** Max amount of time that may elapse since an attack input for the combo to continue */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Combo", meta = (ClampMin = 0, ClampMax = 5, Units = "s"))
	float ComboInputCacheTimeTolerance = 0.45f;

	/** Names of the montage sections that correspond to each stage of the combo attack */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Combo")
	TArray<FName> ComboSectionNames;

protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** Whether the combo should advance to the next section */
	virtual bool WantsNextComboSection() const;

	/** Whether the ability listens for attack input events to buffer the next combo hit */
	virtual bool UsesInputBuffering() const;

	/** Called when the montage notify asks whether to continue the combo string */
	UFUNCTION()
	virtual void OnComboCheckEvent(FGameplayEventData EventData);

	/** Called whenever the attack input is pressed while this ability is running */
	UFUNCTION()
	virtual void OnAttackInputEvent(FGameplayEventData EventData);

	/** Index of the current stage of the combo attack */
	int32 ComboCount = 0;

	/** Time at which the attack input was last pressed */
	float LastAttackInputTime = -1000.0f;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> ComboCheckTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> InputEventTask;
};
