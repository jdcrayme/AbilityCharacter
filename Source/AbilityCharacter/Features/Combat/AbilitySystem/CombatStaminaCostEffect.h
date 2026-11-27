// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "CombatStaminaCostEffect.generated.h"

/**
 *  Instant gameplay effect that drains Stamina.
 *  The amount is supplied by the attacking ability through the Data.StaminaCost
 *  SetByCaller magnitude, so every attack can define its own cost while sharing
 *  this one effect class as its cost gameplay effect.
 */
UCLASS()
class ABILITYCHARACTER_API UCombatStaminaCostEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:

	UCombatStaminaCostEffect();
};
