// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StanceMonitorAbility.h"
#include "AIStanceMonitorAbility.generated.h"

/**
 *  Stance monitor for AI characters: detects the player instead of enemies,
 *  so AI characters enter the combat stance when the player gets close.
 */
UCLASS()
class ABILITYCHARACTER_API UAIStanceMonitorAbility : public UStanceMonitorAbility
{
	GENERATED_BODY()

public:

	UAIStanceMonitorAbility();
};
