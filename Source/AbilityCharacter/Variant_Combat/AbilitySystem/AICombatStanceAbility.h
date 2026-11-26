// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SetStanceAbility.h"
#include "AICombatStanceAbility.generated.h"

/**
 *  Combat stance for AI characters: grants the AI's attack abilities while
 *  the player is in range, revokes them again through the neutral stance.
 */
UCLASS()
class ABILITYCHARACTER_API UAICombatStanceAbility : public USetStanceAbility
{
	GENERATED_BODY()

public:

	UAICombatStanceAbility();
};
