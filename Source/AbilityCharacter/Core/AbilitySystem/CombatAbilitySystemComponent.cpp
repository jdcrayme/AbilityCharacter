// Copyright Epic Games, Inc. All Rights Reserved.

#include "CombatAbilitySystemComponent.h"
#include "Core/AbilitySystem/CombatEffectContext.h"
#include "Core/AbilitySystem/CombatTagGrantEffect.h"
#include "GameplayEffect.h"

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

FActiveGameplayEffectHandle UCombatAbilitySystemComponent::ApplyGrantedTags(const FGameplayTag& SlotTag, const FGameplayTagContainer& Tags)
{
	// the tag slots are server-side state: clients see the applied effects (and
	// their granted tags) through the regular gameplay effect replication
	if (GetOwnerRole() != ROLE_Authority || !SlotTag.IsValid() || Tags.IsEmpty())
	{
		return FActiveGameplayEffectHandle();
	}

	// one slot holds one effect: replacing the slot removes the tags the previous
	// state granted (for example the combat stance's tags when the neutral stance
	// re-applies the State.Stance slot)
	if (FActiveGameplayEffectHandle* ExistingHandle = TagSlotHandles.Find(SlotTag))
	{
		RemoveActiveGameplayEffect(*ExistingHandle);
		TagSlotHandles.Remove(SlotTag);
	}

	const FGameplayEffectSpecHandle SpecHandle = MakeOutgoingSpec(UCombatTagGrantEffect::StaticClass(), 1.f, MakeEffectContext());
	if (SpecHandle.IsValid())
	{
		// the tags travel on the spec, so one effect class serves every combination
		SpecHandle.Data->DynamicGrantedTags.AppendTags(Tags);

		const FActiveGameplayEffectHandle NewHandle = ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		TagSlotHandles.Add(SlotTag, NewHandle);
		return NewHandle;
	}

	return FActiveGameplayEffectHandle();
}

void UCombatAbilitySystemComponent::ClearGrantedTags(const FGameplayTag& SlotTag)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	if (FActiveGameplayEffectHandle* ExistingHandle = TagSlotHandles.Find(SlotTag))
	{
		RemoveActiveGameplayEffect(*ExistingHandle);
		TagSlotHandles.Remove(SlotTag);
	}
}
