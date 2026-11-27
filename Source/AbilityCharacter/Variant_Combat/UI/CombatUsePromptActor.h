// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/WidgetComponent.h"
#include "CombatUsePromptActor.generated.h"

class UUserWidget;

/**
 *  World-space holder for the use prompt, spawned and destroyed by the passive
 *  use ability. Keeping the prompt in its own actor means the ability owns the
 *  whole prompt lifecycle without touching its avatar's component hierarchy.
 *  The prompt smoothly interpolates towards its target transform so the 10 Hz
 *  interaction trace still yields fluid movement.
 */
UCLASS()
class ABILITYCHARACTER_API ACombatUsePromptActor : public AActor
{
	GENERATED_BODY()

public:

	/** Constructor */
	ACombatUsePromptActor();

	/**
	 *  Configures the prompt before the actor finishes spawning (deferred spawn):
	 *  sets the widget class, render space, draw size and follow speed.
	 */
	void SetupPrompt(TSubclassOf<UUserWidget> InWidgetClass, EWidgetSpace InSpace, const FVector2D& InDrawSize, float InSmoothingSpeed);

	/** Returns the prompt widget instance currently rendered, or null */
	UUserWidget* GetPromptWidget() const;

	/**
	 *  Moves the prompt target: the prompt sits at WorldLocation and rotates to
	 *  face FaceLocation (the interacting player) so world-space text stays
	 *  readable. Pass bSnapToTarget to teleport instead of interpolate.
	 */
	void SetPromptTarget(const FVector& WorldLocation, const FVector& FaceLocation, bool bSnapToTarget);

	virtual void Tick(float DeltaSeconds) override;

protected:

	/** Widget component rendering the prompt */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UWidgetComponent> Widget;

private:

	/** Where the prompt is heading */
	FVector TargetLocation = FVector::ZeroVector;

	/** Rotation the prompt is heading towards (facing the player) */
	FRotator TargetRotation = FRotator::ZeroRotator;

	/** Interpolation speed for following the target; 0 snaps every update */
	float SmoothingSpeed = 0.0f;
};
