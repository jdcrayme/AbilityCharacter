// Copyright Epic Games, Inc. All Rights Reserved.

#include "CombatUsePromptActor.h"
#include "Blueprint/UserWidget.h"

ACombatUsePromptActor::ACombatUsePromptActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// hidden until the use ability has a target to show
	SetActorHiddenInGame(true);

	// the widget component is the whole actor
	RootComponent = Widget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PromptWidget"));

	// never collide and never interfere with the interaction ray itself
	Widget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Widget->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Widget->SetGenerateOverlapEvents(false);
}

void ACombatUsePromptActor::SetupPrompt(TSubclassOf<UUserWidget> InWidgetClass, EWidgetSpace InSpace, const FVector2D& InDrawSize, float InSmoothingSpeed)
{
	SmoothingSpeed = InSmoothingSpeed;

	if (Widget)
	{
		Widget->SetWidgetClass(InWidgetClass);
		Widget->SetWidgetSpace(InSpace);
		Widget->SetDrawSize(InDrawSize);

		// world-space quads need transparency to composite over the scene
		Widget->SetBlendMode(EWidgetBlendMode::Transparent);
		Widget->SetBackgroundColor(FLinearColor::Transparent);

		// readable from both sides in world space
		Widget->SetTwoSided(true);
	}
}

UUserWidget* ACombatUsePromptActor::GetPromptWidget() const
{
	return Widget ? Cast<UUserWidget>(Widget->GetUserWidgetObject()) : nullptr;
}

void ACombatUsePromptActor::SetPromptTarget(const FVector& WorldLocation, const FVector& FaceLocation, bool bSnapToTarget)
{
	TargetLocation = WorldLocation;

	// point the widget's front (its +X axis) at the viewer
	TargetRotation = (FaceLocation - WorldLocation).Rotation();

	if (bSnapToTarget || SmoothingSpeed <= 0.0f)
	{
		SetActorLocationAndRotation(TargetLocation, TargetRotation);
	}
}

void ACombatUsePromptActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// nothing to follow while the prompt is hidden
	if (IsHidden())
	{
		return;
	}

	SetActorLocationAndRotation(
		FMath::VInterpTo(GetActorLocation(), TargetLocation, DeltaSeconds, SmoothingSpeed),
		FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaSeconds, SmoothingSpeed));
}
