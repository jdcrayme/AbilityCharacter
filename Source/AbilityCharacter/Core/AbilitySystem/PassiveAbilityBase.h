// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "PassiveAbilityBase.generated.h"

/**
 *  Base class for passive abilities: granted with the default abilities, they
 *  activate on their own on the server and then run for the owner's lifetime
 *  (the stance monitors, the use ability...).
 *
 *  Replaces the grant-means-activate block that was copy-pasted into every
 *  passive ability. Deriving classes configure their behavior as usual and
 *  clean up their timers/tasks in EndAbility.
 */
UCLASS(Abstract)
class ABILITYCHARACTER_API UPassiveAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()

public:

	UPassiveAbilityBase();

protected:

	/**
	 *  Granting alone does not activate an ability: passive abilities start
	 *  themselves as soon as they are granted (server side).
	 */
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
};
