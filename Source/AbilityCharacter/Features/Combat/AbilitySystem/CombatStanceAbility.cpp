// Copyright Epic Games, Inc. All Rights Reserved.

#include "CombatStanceAbility.h"
#include "Core/AbilitySystem/CombatGameplayTags.h"
#include "Features/Combat/AbilitySystem/ComboAttackAbility.h"
#include "Features/Combat/AbilitySystem/ChargedAttackAbility.h"

UCombatStanceAbility::UCombatStanceAbility()
{
	SetAssetTags(FGameplayTagContainer(TAG_Ability_Stance_Combat));

	StanceTagsToAdd.AddTag(TAG_State_Stance_Combat);

	// entering combat arms the attack abilities
	AbilitiesToGrant = {
		UComboAttackAbility::StaticClass(),
		UChargedAttackAbility::StaticClass()
	};
}
