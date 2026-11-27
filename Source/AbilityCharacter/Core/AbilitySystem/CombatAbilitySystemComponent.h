// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CombatAbilitySystemComponent.generated.h"

/**
 *  Ability system component used by all combat actors.
 *  Allocates FCombatGameplayEffectContext so every effect applied through this
 *  component can carry combat data (hit results, knockback impulses).
 */
UCLASS()
class ABILITYCHARACTER_API UCombatAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:

	virtual FGameplayEffectContextHandle MakeEffectContext() const override;
};
