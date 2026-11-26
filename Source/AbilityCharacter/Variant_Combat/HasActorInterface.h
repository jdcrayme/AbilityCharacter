// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HasActorInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UHasActorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ABILITYCHARACTER_API IHasActorInterface
{
	GENERATED_BODY()

public:
	// BlueprintNativeEvent allows implementation in both C++ and Blueprints
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Actor Interface")
	void SetActor(AActor* NewActor);
};
