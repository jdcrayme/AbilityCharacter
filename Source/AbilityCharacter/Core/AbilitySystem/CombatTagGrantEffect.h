// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "CombatTagGrantEffect.generated.h"

/**
 *  Infinite gameplay effect used by UCombatAbilitySystemComponent::ApplyGrantedTags
 *  to grant gameplay tags at runtime.
 *
 *  Carries no modifiers and no tags of its own: the tags come from the effect
 *  spec's DynamicGrantedTags, so this single class serves every tag combination
 *  (stances, equipment states, weapon modes...).
 *
 *  Effect-granted tags replicate to every client (unlike loose tags, which are
 *  server-only) and are removed with their effect, which is what makes them the
 *  right carrier for state tags that animation blueprints and UI need to see -
 *  for example State.Stance.* or, later, State.Weapon.*.
 */
UCLASS()
class ABILITYCHARACTER_API UCombatTagGrantEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:

	UCombatTagGrantEffect();
};
