// Copyright Epic Games, Inc. All Rights Reserved.

#include "CombatAbilitySystemComponent.h"
#include "Core/AbilitySystem/CombatEffectContext.h"

FGameplayEffectContextHandle UCombatAbilitySystemComponent::MakeEffectContext() const
{
	FGameplayEffectContextHandle Context = FGameplayEffectContextHandle(new FCombatGameplayEffectContext());

	// By default use the owner and avatar as the instigator and causer
	if (ensureMsgf(AbilityActorInfo.IsValid(), TEXT("Unable to make effect context because AbilityActorInfo is not valid.")))
	{
		Context.AddInstigator(AbilityActorInfo->OwnerActor.Get(), AbilityActorInfo->AvatarActor.Get());
	}

	return Context;
}
