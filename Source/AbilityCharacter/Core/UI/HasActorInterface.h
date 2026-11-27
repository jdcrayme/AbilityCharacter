// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HasActorInterface.generated.h"

UINTERFACE(MinimalAPI)
class UHasActorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *  Interface for widgets that need to know which actor they represent.
 *  UActorWidgetComponent calls SetActor on the widget it spawns, so
 *  actor-driven widgets (life bars, prompts...) receive their actor without
 *  Blueprint glue.
 */
class ABILITYCHARACTER_API IHasActorInterface
{
	GENERATED_BODY()

public:

	/** Passes the actor the widget should represent; implementable in C++ and Blueprints */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Actor Interface")
	void SetActor(AActor* NewActor);
};
