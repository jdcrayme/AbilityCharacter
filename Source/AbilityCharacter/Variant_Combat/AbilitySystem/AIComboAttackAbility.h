// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ComboAttackAbility.h"
#include "AIComboAttackAbility.generated.h"

/**
 *  AI version of the combo attack: instead of buffering player input the enemy
 *  picks a random number of hits up front, matching the original StateTree behaviour.
 */
UCLASS()
class ABILITYCHARACTER_API UAIComboAttackAbility : public UComboAttackAbility
{
	GENERATED_BODY()

public:

	UAIComboAttackAbility();

protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual bool WantsNextComboSection() const override;
	virtual bool UsesInputBuffering() const override;

	/** How many hits this combo string will play */
	int32 TargetComboCount = 0;
};
