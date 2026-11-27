// Copyright Epic Games, Inc. All Rights Reserved.

#include "CombatPlayerController.h"
#include "Features/Combat/CombatCharacter.h"
#include "Engine/World.h"

void ACombatPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// subscribe to the pawn's OnDestroyed delegate
	InPawn->OnDestroyed.AddDynamic(this, &ACombatPlayerController::OnPawnDestroyed);
}

void ACombatPlayerController::SetRespawnTransform(const FTransform& NewRespawn)
{
	// save the new respawn transform
	RespawnTransform = NewRespawn;
}

void ACombatPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	// spawn a new character at the respawn transform
	if (ACombatCharacter* RespawnedCharacter = GetWorld()->SpawnActor<ACombatCharacter>(CharacterClass, RespawnTransform))
	{
		// possess the character
		Possess(RespawnedCharacter);
	}
}
