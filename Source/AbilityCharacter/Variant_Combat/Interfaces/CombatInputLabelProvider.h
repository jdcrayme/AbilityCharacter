// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "CombatInputLabelProvider.generated.h"

/**
 *  Interface for pawns that can turn an ability input event tag into the key
 *  label bound to it (e.g. "E" for Combat.Input.UsePressed), so prompts and
 *  other UI can display the right key without hard-coding it.
 *  ACharacterBase implements this from its EventInputMappings table; any pawn
 *  forwarding ability input events can implement it to feed prompts its own
 *  key labels (including from Enhanced Input user settings on engine versions
 *  that support querying them).
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UCombatInputLabelProvider : public UInterface
{
	GENERATED_BODY()
};

class ABILITYCHARACTER_API ICombatInputLabelProvider
{
	GENERATED_BODY()

public:

	/** Returns the key label bound to the event tag, or an empty text when unknown; empty lets the caller fall back to its own default */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Input")
	FText GetKeyLabelForEvent(FGameplayTag EventTag);
};
