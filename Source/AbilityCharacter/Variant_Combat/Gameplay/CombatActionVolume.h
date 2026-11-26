// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatUsable.h"
#include "CombatActionVolume.generated.h"

class UBoxComponent;

/** Broadcast when the volume is used; passes the actor that used the volume */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionVolumeUsed, AActor*, Instigator);

/**
 *  An invisible action volume the passive use ability's interaction ray can find.
 *  When a player looks at it and presses the use key, the volume:
 *  - broadcasts OnUsed (Blueprints and StateTree tasks can listen for it),
 *  - activates every CombatActivatable actor in ActorsToActivate, reusing the
 *    existing activation interface.
 *  The box only responds to the visibility trace channel, so characters walk
 *  straight through it while look-at traces hit it.
 */
UCLASS()
class ABILITYCHARACTER_API ACombatActionVolume : public AActor, public ICombatUsable
{
	GENERATED_BODY()

	/** Collision box the interaction ray casts against */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* Box;

public:

	/** Constructor */
	ACombatActionVolume();

	/** Called when the volume is used; passes the actor that used it */
	UPROPERTY(BlueprintAssignable, Category="Action Volume")
	FOnActionVolumeUsed OnUsed;

	/** Action text shown in the use prompt, e.g. "Open" or "Talk" */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Action Volume")
	FText UseText;

	/** Actors activated when the volume is used, through the CombatActivatable interface */
	UPROPERTY(EditAnywhere, Category="Action Volume")
	TArray<AActor*> ActorsToActivate;

	/** Whether the volume can only be used once */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Action Volume")
	bool bSingleUse = false;

	// ~begin CombatUsable interface

	virtual bool CanUse_Implementation(AActor* Instigator) override;
	virtual FText GetUseText_Implementation(AActor* Instigator) override;
	virtual void Use_Implementation(AActor* Instigator) override;

	// ~end CombatUsable interface

protected:

	/** True after a single-use volume has been used */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Action Volume")
	bool bUsed = false;
};
