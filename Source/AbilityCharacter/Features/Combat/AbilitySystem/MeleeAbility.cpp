// Copyright Epic Games, Inc. All Rights Reserved.

#include "MeleeAbility.h"

UMeleeAbility::UMeleeAbility()
{
        // players want their attack montages predicted: the montage plays locally
        // while the server owns the damage (CombatAbilityBase gates the damage
        // paths on authority). AI instances only ever run server-side.
        NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}
