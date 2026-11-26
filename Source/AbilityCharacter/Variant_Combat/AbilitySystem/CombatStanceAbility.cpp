// Copyright Epic Games, Inc. All Rights Reserved.

#include "CombatStanceAbility.h"
#include "CombatGameplayTags.h"
#include "ComboAttackAbility.h"
#include "ChargedAttackAbility.h"

UCombatStanceAbility::UCombatStanceAbility()
{
	AbilityTags.AddTag(TAG_Ability_Stance_Combat);

	StanceTagsToAdd.AddTag(TAG_State_Stance_Combat);

	// entering combat arms the attack abilities
	AbilitiesToGrant = {
		UComboAttackAbility::StaticClass(),
		UChargedAttackAbility::StaticClass()
	};
}
