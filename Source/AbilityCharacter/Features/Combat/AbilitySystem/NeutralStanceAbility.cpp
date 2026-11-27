// Copyright Epic Games, Inc. All Rights Reserved.

#include "NeutralStanceAbility.h"
#include "Core/AbilitySystem/CombatGameplayTags.h"
#include "Features/Combat/AbilitySystem/ComboAttackAbility.h"
#include "Features/Combat/AbilitySystem/ChargedAttackAbility.h"
#include "Features/Combat/AbilitySystem/AIComboAttackAbility.h"
#include "Features/Combat/AbilitySystem/AIChargedAttackAbility.h"

UNeutralStanceAbility::UNeutralStanceAbility()
{
	SetAssetTags(FGameplayTagContainer(TAG_Ability_Stance_Neutral));

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
