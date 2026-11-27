// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatUsePromptInterface.generated.h"

/**
 *  Interface for use prompt widgets rendered by the passive use ability.
 *  The ability works exclusively through this interface: any UserWidget class
 *  (C++ or pure Blueprint) implementing it can be assigned as the ability's
 *  prompt widget, so the ability never depends on a concrete widget class.
 *  The action text originates from the usable target (e.g. an action volume
 *  component) and is forwarded to the widget on every interaction trace.
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UCombatUsePromptInterface : public UInterface
{
	GENERATED_BODY()
};

class ABILITYCHARACTER_API ICombatUsePromptInterface
{
	GENERATED_BODY()

public:

	/** Updates the prompt: the key label (e.g. "E") and the action text from the usable target (e.g. "Open") */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Use Prompt")
	void UpdatePromptText(const FText& KeyText, const FText& ActionText);
};
