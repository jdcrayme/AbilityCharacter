// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AbilityCharacterPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;

/**
 *  Shared base PlayerController for the feature player controllers
 *  (combat, platforming, side scrolling).
 *  Adds the Enhanced Input mapping contexts to the local player, optionally
 *  spawns UMG touch controls on mobile platforms, and offers a Player Start
 *  respawn helper. Subclasses add feature-specific behaviour.
 */
UCLASS(abstract, Config="Game")
class AAbilityCharacterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input Mapping Contexts added to the local player on setup */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts skipped while touch controls are active */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

	/** Spawns PawnClass at the first Player Start in the level and possesses it; returns the spawned pawn, or null */
	APawn* RespawnPawnAtPlayerStart(TSubclassOf<APawn> PawnClass);
};
