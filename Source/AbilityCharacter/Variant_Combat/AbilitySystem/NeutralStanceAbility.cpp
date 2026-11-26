// Copyright Epic Games, Inc. All Rights Reserved.

#include "NeutralStanceAbility.h"
#include "CombatGameplayTags.h"
#include "ComboAttackAbility.h"
#include "ChargedAttackAbility.h"
#include "AIComboAttackAbility.h"
#include "AIChargedAttackAbility.h"

UNeutralStanceAbility::UNeutralStanceAbility()
{
	AbilityTags.AddTag(TAG_Ability_Stance_Neutral);

	StanceTagsToRemove.AddTag(TAG_State_Stance_Combat);

	// leaving combat disarms the attack abilities (player and AI variants;
	// revoking an ungranted ability is a no-op)
	AbilitiesToRevoke = {
		UComboAttackAbility::StaticClass(),
		UChargedAttackAbility::StaticClass(),
		UAIComboAttackAbility::StaticClass(),
		UAIChargedAttackAbility::StaticClass()
	};
}
