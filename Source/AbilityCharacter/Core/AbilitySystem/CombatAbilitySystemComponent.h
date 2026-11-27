// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CombatAbilitySystemComponent.generated.h"

/**
 *  Ability system component used by all combat actors.
 *  Allocates FCombatGameplayEffectContext so every effect applied through this
 *  component can carry combat data (hit results, knockback impulses), and
 *  manages replicated tag slots.
 */
UCLASS()
class ABILITYCHARACTER_API UCombatAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:

	virtual FGameplayEffectContextHandle MakeEffectContext() const override;

	/**
	 *  Grants Tags to the owner through an infinite UCombatTagGrantEffect, replacing
	 *  whatever was previously granted under SlotTag. Effect-granted tags replicate to
	 *  every client and are removed again by ClearGrantedTags, unlike loose tags which
	 *  are server-only and never cleaned up.
	 *  The slot tag groups a feature's state tags: State.Stance for stances, later
	 *  State.Weapon for equipment - re-applying a slot is how a previous state's tags
	 *  are swapped out (combat stance -> neutral stance).
	 *  Server only; returns an invalid handle when called on a client or with no tags.
	 */
	FActiveGameplayEffectHandle ApplyGrantedTags(const FGameplayTag& SlotTag, const FGameplayTagContainer& Tags);

	/** Removes the effect currently granting a slot's tags; server only */
	void ClearGrantedTags(const FGameplayTag& SlotTag);

private:

	/** Tag slot -> handle of the effect currently granting that slot's tags (server-side bookkeeping) */
	TMap<FGameplayTag, FActiveGameplayEffectHandle> TagSlotHandles;
};
