// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CombatUsePromptInterface.h"
#include "CombatUsePromptWidget.generated.h"

class UTextBlock;

/**
 *  "Press [key] to use" prompt shown by the passive use ability while a usable
 *  actor is in range.
 *  Builds a default "[E] Open" layout in C++ so it works without any assets;
 *  Blueprint subclasses can design their own layout and react to
 *  BP_OnPromptTextChanged instead of using the built-in one.
 *  The ability drives the widget through the CombatUsePromptInterface, so any
 *  other widget class implementing that interface can take this one's place.
 */
UCLASS()
class ABILITYCHARACTER_API UCombatUsePromptWidget : public UUserWidget, public ICombatUsePromptInterface
{
	GENERATED_BODY()

public:

	/** Updates the key and action labels shown by the prompt */
	UFUNCTION(BlueprintCallable, Category="Use Prompt")
	void SetPromptText(const FText& KeyText, const FText& ActionText);

	// ~begin CombatUsePromptInterface interface

	/** Interface entry point used by the use ability: forwards to SetPromptText */
	virtual void UpdatePromptText_Implementation(const FText& KeyText, const FText& ActionText) override;

	// ~end CombatUsePromptInterface interface

	/** Blueprint hook for custom prompt layouts: called whenever the prompt text changes */
	UFUNCTION(BlueprintImplementableEvent, Category="Use Prompt")
	void BP_OnPromptTextChanged(const FText& KeyText, const FText& ActionText);

protected:

	virtual void NativeOnInitialized() override;

	/** Builds the default "[Key] Action" layout when the widget has no designed tree */
	void BuildDefaultLayout();

	/** Text block showing the use key, e.g. "E" */
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> KeyTextBlock;

	/** Text block showing the action, e.g. "Open" */
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ActionTextBlock;
};
