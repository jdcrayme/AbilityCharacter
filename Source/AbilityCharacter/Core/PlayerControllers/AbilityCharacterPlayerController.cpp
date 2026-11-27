// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilityCharacterPlayerController.h"

#include "AbilityCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerStart.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Input/SVirtualJoystick.h"

void AAbilityCharacterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogAbilityCharacter, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AAbilityCharacterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// add the input mapping context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

bool AAbilityCharacterPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

APawn* AAbilityCharacterPlayerController::RespawnPawnAtPlayerStart(TSubclassOf<APawn> PawnClass)
{
	// find the first player start in the level
	TArray<AActor*> ActorList;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), ActorList);

	if (ActorList.Num() == 0)
	{
		UE_LOG(LogAbilityCharacter, Warning, TEXT("Could not respawn pawn: no Player Start found in the level."));
		return nullptr;
	}

	// spawn the pawn at the player start and possess it
	const FTransform SpawnTransform = ActorList[0]->GetActorTransform();
	if (APawn* RespawnedPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform))
	{
		Possess(RespawnedPawn);
		return RespawnedPawn;
	}

	return nullptr;
}
