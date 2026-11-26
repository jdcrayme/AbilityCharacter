// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "SetStanceAbility.generated.h"

/**
 *  Base ability for character stances (Combat, Walk, Run, Sneak, Crouch, Prone...).
 *  A stance is expressed through gameplay tags on the owner's ability system
 *  component (State.Stance.*) plus an optional granted gameplay effect for
 *  attribute-driven changes such as movement speed.
 *
 *  Stances can also grant a set of abilities while they are active (for example
 *  the combat stance grants the primary and secondary attacks) and revoke a set
 *  of previously granted abilities (for example leaving the combat stance).
 *
 *  Stance abilities gate which attack abilities can activate, drive anim set
 *  changes via the tags, and can apply effects while the stance is active.
 */
UCLASS()
class ABILITYCHARACTER_API USetStanceAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:

	USetStanceAbility();

	/** Stance tags added to the owner when this ability activates */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Stance")
	FGameplayTagContainer StanceTagsToAdd;

	/** Stance tags removed from the owner when this ability activates */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Stance")
	FGameplayTagContainer StanceTagsToRemove;

	/** Abilities granted to the owner while this stance is active (skipped if already granted) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Stance")
	TArray<TSubclassOf<UGameplayAbility>> AbilitiesToGrant;

	/** Abilities revoked from the owner when this stance activates (for example attacks revoked when leaving the combat stance) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Stance")
	TArray<TSubclassOf<UGameplayAbility>> AbilitiesToRevoke;

	/** Optional effect applied while the stance is active (e.g. a movement speed modifier) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Stance")
	TSubclassOf<UGameplayEffect> StanceEffect;

protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** Revokes every granted ability of the given class */
	void RevokeAbilitiesOfClass(TSubclassOf<UGameplayAbility> AbilityClass);

	/** Returns true if the owner already has an ability of the given class granted */
	bool HasAbilityOfClass(TSubclassOf<UGameplayAbility> AbilityClass) const;

private:

	/** Handle of the currently applied stance effect, so it can be removed when the stance changes */
	FActiveGameplayEffectHandle StanceEffectHandle;
};
