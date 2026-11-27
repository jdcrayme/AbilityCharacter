// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatActionVolume.generated.h"

class UBoxComponent;
class UCombatActionVolumeComponent;

/**
 *  An invisible action volume the passive use ability's interaction ray can
 *  find: a traceable box plus a UCombatActionVolumeComponent that carries the
 *  interaction (UseText, OnUsed, linked activatable actors, single use).
 *  Configure the interaction on the Volume component.
 *  To make an existing actor interactive instead (a vehicle, a door...), add
 *  the UCombatActionVolumeComponent to it; this actor is only the standalone,
 *  level-placeable flavor.
 *  The box only responds to the visibility trace channel, so characters walk
 *  straight through it while look-at traces hit it.
 */
UCLASS()
class ABILITYCHARACTER_API ACombatActionVolume : public AActor
{
	GENERATED_BODY()

	/** Collision box the interaction ray casts against */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> Box;

public:

	/** Constructor */
	ACombatActionVolume();

	/** The action volume component implementing the CombatUsable interaction */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UCombatActionVolumeComponent> Volume;
};
