// Copyright Epic Games, Inc. All Rights Reserved.

#include "CombatActionVolume.h"
#include "CombatActivatable.h"
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

	// default prompt text
	UseText = NSLOCTEXT("CombatActionVolume", "DefaultUseText", "Use");
}

bool ACombatActionVolume::CanUse_Implementation(AActor* )
{
	// single-use volumes stop being usable once they have been used
	return !bSingleUse || !bUsed;
}

FText ACombatActionVolume::GetUseText_Implementation(AActor* )
{
	return UseText;
}

void ACombatActionVolume::Use_Implementation(AActor* User)
{
	if (bSingleUse)
	{
		bUsed = true;
	}

	UE_LOG(LogTemp, Warning, TEXT("[ActionVolume] %s was used by %s"), *GetName(), User ? *User->GetName() : TEXT("nobody"));

	// notify listeners (Blueprints, StateTree, UI...)
	OnUsed.Broadcast(User);

	// activate the linked actors through the existing activation interface
	for (AActor* ActorToActivate : ActorsToActivate)
	{
		if (ICombatActivatable* Activatable = Cast<ICombatActivatable>(ActorToActivate))
		{
			Activatable->ActivateInteraction(User);
		}
	}
}
