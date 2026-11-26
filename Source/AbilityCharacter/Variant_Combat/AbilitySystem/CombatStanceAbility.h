// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SetStanceAbility.h"
#include "CombatStanceAbility.generated.h"

/**
 *  Combat stance: applied when the character is in a fight (an enemy is nearby
 *  or the character walks into a combat arena, see UStanceMonitorAbility).
 *  Adds the State.Stance.Combat tag and grants the primary and secondary attack
 *  abilities; the neutral stance revokes them again.
 */
UCLASS()
class ABILITYCHARACTER_API UCombatStanceAbility : public USetStanceAbility
{
	GENERATED_BODY()

public:

	UCombatStanceAbility();
};
