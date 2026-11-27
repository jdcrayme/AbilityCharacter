// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/PlayerControllers/AbilityCharacterPlayerController.h"
#include "CombatPlayerController.generated.h"

class ACombatCharacter;

/**
 *  Player Controller for the combat game mode.
 *  Input mapping contexts and touch controls come from
 *  AAbilityCharacterPlayerController; this class adds checkpoint-based
 *  respawning of the player character.
 */
UCLASS(abstract, Config="Game")
class ABILITYCHARACTER_API ACombatPlayerController : public AAbilityCharacterPlayerController
{
	GENERATED_BODY()
	
protected:

	/** Character class to respawn when the possessed pawn is destroyed */
	UPROPERTY(EditAnywhere, Category="Respawn")
	TSubclassOf<ACombatCharacter> CharacterClass;

	/** Transform to respawn the character at. Can be set to create checkpoints */
	FTransform RespawnTransform;

	/** Pawn initialization */
	virtual void OnPossess(APawn* InPawn) override;

public:

	/** Updates the character respawn transform */
	void SetRespawnTransform(const FTransform& NewRespawn);

protected:

	/** Called if the possessed pawn is destroyed */
	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedActor);
};
