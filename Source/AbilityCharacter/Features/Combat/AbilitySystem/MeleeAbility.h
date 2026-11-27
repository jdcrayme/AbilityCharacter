// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Features/Combat/AbilitySystem/CombatAbilityBase.h"
#include "MeleeAbility.generated.h"

/**
 *  Base class for melee attack abilities.
 *  Hit detection is driven by Event.AttackTrace gameplay events fired from the
 *  montage's AnimNotify_DoAttackTrace, so weapons and attacks can place traces
 *  exactly where their animation strikes.
 */
UCLASS()
class ABILITYCHARACTER_API UMeleeAbility : public UCombatAbilityBase
{
	GENERATED_BODY()

public:

	UMeleeAbility();
};
