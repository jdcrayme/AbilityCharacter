// Copyright Epic Games, Inc. All Rights Reserved.

#include "AIComboAttackAbility.h"

UAIComboAttackAbility::UAIComboAttackAbility()
{
	// enemy tuning: matches the previous combat enemy configuration
	AttackDamage = 1.0f;
	KnockbackImpulse = 150.0f;
	LaunchImpulse = 350.0f;
	MeleeTraceRadius = 50.0f;

	// AI attacks only hit pawns, they don't damage destructibles
	bDamageWorldObjects = false;
}

void UAIComboAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// choose how many times we're going to attack before playing the montage
	TargetComboCount = FMath::RandRange(1, FMath::Max(1, ComboSectionNames.Num() - 1));

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

bool UAIComboAttackAbility::WantsNextComboSection() const
{
	// keep playing the string until we hit the number of attacks we rolled
	return (ComboCount + 1) < TargetComboCount;
}

bool UAIComboAttackAbility::UsesInputBuffering() const
{
	return false;
}
