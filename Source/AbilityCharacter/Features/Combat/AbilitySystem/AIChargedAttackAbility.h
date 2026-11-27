// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Features/Combat/AbilitySystem/ChargedAttackAbility.h"
#include "AIChargedAttackAbility.generated.h"

/**
 *  AI version of the charged attack: instead of following the input the enemy
 *  charges for a random number of loops, matching the original StateTree behaviour.
 */
UCLASS()
class ABILITYCHARACTER_API UAIChargedAttackAbility : public UChargedAttackAbility
{
	GENERATED_BODY()

public:

	UAIChargedAttackAbility();

protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual bool ShouldReleaseCharge() override;

	/** Minimum number of charge animation loops that will be played */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Charged", meta = (ClampMin = 1, ClampMax = 20))
	int32 MinChargeLoops = 2;

	/** Maximum number of charge animation loops that will be played */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Charged", meta = (ClampMin = 1, ClampMax = 20))
	int32 MaxChargeLoops = 5;

	/** Target number of charge loops for this charged attack */
	int32 TargetChargeLoops = 0;

	/** Number of charge loops played so far */
	int32 CurrentChargeLoop = 0;
};
