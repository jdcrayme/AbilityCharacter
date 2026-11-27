// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "ActorWidgetComponent.generated.h"

/**
 *  Widget component that hands its owning actor to the widget it spawns.
 *  When the widget class implements HasActorInterface, the component calls
 *  SetActor(owner) right after the widget is initialized, so widgets such as
 *  life bars can bind to their actor without any Blueprint glue.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ABILITYCHARACTER_API UActorWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
public:

	/** Passes the owning actor to widgets implementing the HasActorInterface */
	virtual void InitWidget() override;
};
