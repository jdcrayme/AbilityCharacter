// Copyright Epic Games, Inc. All Rights Reserved.

#include "AIChargedAttackAbility.h"

UAIChargedAttackAbility::UAIChargedAttackAbility()
{
	// enemy tuning: matches the previous combat enemy configuration
	AttackDamage = 1.0f;
	KnockbackImpulse = 150.0f;
	LaunchImpulse = 350.0f;
	MeleeTraceRadius = 50.0f;
	bDamageWorldObjects = false;
}

void UAIChargedAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// choose how many loops we're going to charge for before playing the montage
	TargetChargeLoops = FMath::RandRange(MinChargeLoops, MaxChargeLoops);
	CurrentChargeLoop = 0;

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

bool UAIChargedAttackAbility::ShouldReleaseCharge()
{
	// the AI releases the charge once it has looped enough times
	++CurrentChargeLoop;

	return CurrentChargeLoop >= TargetChargeLoops;
}
