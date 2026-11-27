// Copyright Epic Games, Inc. All Rights Reserved.

#include "ActorWidgetComponent.h"
#include "Core/UI/HasActorInterface.h"

void UActorWidgetComponent::InitWidget()
{
	Super::InitWidget();
	
	// get the actual UUserWidget instance
	UUserWidget* CurrentWidget = GetUserWidgetObject();

	if (CurrentWidget)
	{
		// check whether the widget class implements the interface
		// (the U-prefixed class is used for the check)
		if (CurrentWidget->GetClass()->ImplementsInterface(UHasActorInterface::StaticClass()))
		{
			IHasActorInterface::Execute_SetActor(CurrentWidget, GetOwner());
		}
	}
}
