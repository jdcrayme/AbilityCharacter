// Copyright Epic Games, Inc. All Rights Reserved.

#include "CombatStaminaCostEffect.h"
#include "CombatGameplayTags.h"
#include "CharacterAttributeSet.h"

UCombatStaminaCostEffect::UCombatStaminaCostEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat SetByCaller;
	SetByCaller.DataTag = TAG_Data_StaminaCost;

	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UCharacterAttributeSet::GetStaminaAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	Modifiers.Add(Modifier);
}
