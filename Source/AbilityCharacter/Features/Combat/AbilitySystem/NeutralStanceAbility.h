// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Features/Combat/AbilitySystem/SetStanceAbility.h"
#include "NeutralStanceAbility.generated.h"

/**
 *  Neutral stance: applied when no enemies are around. Removes the combat
 *  stance tag and revokes the combat attack abilities.
 */
UCLASS()
class ABILITYCHARACTER_API UNeutralStanceAbility : public USetStanceAbility
{
	GENERATED_BODY()

public:

	UNeutralStanceAbility();
};
