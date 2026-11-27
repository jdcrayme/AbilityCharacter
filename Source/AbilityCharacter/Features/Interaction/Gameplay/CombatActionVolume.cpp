// Copyright Epic Games, Inc. All Rights Reserved.

#include "CombatActionVolume.h"
#include "Features/Interaction/Gameplay/CombatActionVolumeComponent.h"
#include "Components/BoxComponent.h"

ACombatActionVolume::ACombatActionVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	// create the box volume
	RootComponent = Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	check(Box);

	// set the box's extent
	Box->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));

	// invisible to movement, but the use ability's interaction ray must find it:
	// the box only responds to the visibility trace channel, so characters walk
	// straight through it while look-at traces hit it
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionObjectType(ECC_WorldDynamic);
	Box->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);

	// the volume itself is invisible in game; the editor still draws its bounds
	Box->SetHiddenInGame(true);

	// the interaction lives on the component so any actor can host one too;
	// this actor's box already serves as the trace target
	Volume = CreateDefaultSubobject<UCombatActionVolumeComponent>(TEXT("Volume"));
	Volume->bCreateTraceBox = false;
}
