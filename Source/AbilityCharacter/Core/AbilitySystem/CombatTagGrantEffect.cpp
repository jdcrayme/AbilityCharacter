// Copyright Epic Games, Inc. All Rights Reserved.

#include "CombatTagGrantEffect.h"

UCombatTagGrantEffect::UCombatTagGrantEffect()
{
	// the tags stay granted until the slot effect is explicitly removed
	DurationPolicy = EGameplayEffectDurationType::Infinite;
}
