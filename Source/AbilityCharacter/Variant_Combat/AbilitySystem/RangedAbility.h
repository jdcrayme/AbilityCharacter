// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CombatAbilityBase.h"
#include "RangedAbility.generated.h"

/**
 *  Base class for ranged attack abilities (a pistol, for example).
 *  Fires a hitscan trace from the avatar's view or the weapon muzzle socket on
 *  activation, then follows the same effect -> cue chain as melee attacks.
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

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
