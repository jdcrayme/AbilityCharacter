// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorWidgetComponent.h"
#include "HasActorInterface.h"

void UActorWidgetComponent::InitWidget()
{
	Super::InitWidget();
	
	// Get the actual UUserWidget instance
	UUserWidget* CurrentWidget = GetUserWidgetObject();
 
	if (CurrentWidget)
	{
		// Check if the widget class implements the interface
		// We use the U-prefixed class for the check
		if (CurrentWidget->GetClass()->ImplementsInterface(UHasActorInterface::StaticClass()))
		{
			IHasActorInterface::Execute_SetActor(CurrentWidget, GetOwner());
		}
	}
}