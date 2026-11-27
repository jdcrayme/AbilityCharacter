// Copyright Epic Games, Inc. All Rights Reserved.

#include "CombatEffectContext.h"

#include "GameplayEffect.h"

bool FCombatGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	if (!Super::NetSerialize(Ar, Map, bOutSuccess))
	{
		return false;
	}

	Ar << KnockbackImpulse;

	return true;
}

const FCombatGameplayEffectContext* CombatEffectContext::Extract(const FGameplayEffectContextHandle& Handle)
{
	if (!Handle.IsValid())
	{
		return nullptr;
	}

	return static_cast<const FCombatGameplayEffectContext*>(Handle.Get());
}
