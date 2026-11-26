// Copyright Epic Games, Inc. All Rights Reserved.

#include "AICombatStanceAbility.h"
#include "CombatGameplayTags.h"
#include "AIComboAttackAbility.h"
#include "AIChargedAttackAbility.h"

UAICombatStanceAbility::UAICombatStanceAbility()
{
	AbilityTags.AddTag(TAG_Ability_Stance_Combat);

	StanceTagsToAdd.AddTag(TAG_State_Stance_Combat);

	// entering combat arms the AI's attack abilities
	AbilitiesToGrant = {
		UAIComboAttackAbility::StaticClass(),
		UAIChargedAttackAbility::StaticClass()
	};
}
