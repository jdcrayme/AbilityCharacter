// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatUsable.generated.h"

/**
 *  Usable interface for actors the passive use ability can interact with, such as
 *  action volumes.
 *  Unlike the activatable interface this one is Blueprintable, so interaction
 *  behaviour (doors, levers, NPCs...) can also be implemented in Blueprints.
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UCombatUsable : public UInterface
{
	GENERATED_BODY()
};

/**
 *  Actors implementing this interface can be found by the use ability's
 *  interaction ray and used with the use input.
 */
class ABILITYCHARACTER_API ICombatUsable
{
	GENERATED_BODY()

public:

	/** Whether the actor can currently be used by the instigator */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Usable")
	bool CanUse(AActor* Instigator);

	/** Action text shown next to the key in the use prompt, e.g. "Open" or "Talk" */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Usable")
	FText GetUseText(AActor* Instigator);

	/** Uses the actor with the provided instigator */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Usable")
	void Use(AActor* Instigator);
};
