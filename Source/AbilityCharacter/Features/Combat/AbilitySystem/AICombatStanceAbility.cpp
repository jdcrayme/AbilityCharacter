// Copyright Epic Games, Inc. All Rights Reserved.

#include "AICombatStanceAbility.h"
#include "Core/AbilitySystem/CombatGameplayTags.h"
#include "Features/Combat/AbilitySystem/AIComboAttackAbility.h"
#include "Features/Combat/AbilitySystem/AIChargedAttackAbility.h"

UAICombatStanceAbility::UAICombatStanceAbility()
{
	SetAssetTags(FGameplayTagContainer(TAG_Ability_Stance_Combat));

	StanceTagsToAdd.AddTag(TAG_State_Stance_Combat);

	// entering combat arms the AI's attack abilities
	AbilitiesToGrant = {
		UAIComboAttackAbility::StaticClass(),
		UAIChargedAttackAbility::StaticClass()
	};
}
