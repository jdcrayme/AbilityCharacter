// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Features/Interaction/Interfaces/CombatUsable.h"
#include "CombatActionVolumeComponent.generated.h"

class UBoxComponent;

/** Broadcast when the volume is used; passes the actor that used the volume */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionVolumeUsed, AActor*, Instigator);

/**
 *  Action volume as an actor component: makes any actor (vehicles, doors,
 *  NPCs, levers...) usable by the passive use ability.
 *  The component implements the CombatUsable interface, so when the ability's
 *  interaction ray hits the owning actor, the component answers the CanUse /
 *  GetUseText / Use calls:
 *  - Use broadcasts OnUsed (Blueprints and StateTree tasks can listen for it),
 *  - Use activates every CombatActivatable actor in ActorsToActivate,
 *  - UseText is shown in the use prompt.
 *  By default the component also creates an invisible box on the owner that
 *  blocks the ability's interaction ray; uncheck bCreateTraceBox when the
 *  owner's own collision should serve as the trace target (e.g. a vehicle
 *  mesh) or when a CombatActionVolume actor provides the box.
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class ABILITYCHARACTER_API UCombatActionVolumeComponent : public UActorComponent, public ICombatUsable
{
	GENERATED_BODY()

public:

	/** Constructor */
	UCombatActionVolumeComponent();

	/** Called when the volume is used; passes the actor that used it */
	UPROPERTY(BlueprintAssignable, Category="Action Volume")
	FOnActionVolumeUsed OnUsed;

	/** Action text shown in the use prompt, e.g. "Open" or "Drive" */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Action Volume")
	FText UseText;

	/** Actors activated when the volume is used, through the CombatActivatable interface */
	UPROPERTY(EditAnywhere, Category="Action Volume")
	TArray<TObjectPtr<AActor>> ActorsToActivate;

	/** Whether the volume can only be used once */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Action Volume")
	bool bSingleUse = false;

	/**
	 *  Create an invisible box on the owner that the interaction ray can hit.
	 *  Uncheck when the owner's own collision blocks the trace (e.g. a vehicle
	 *  mesh) or when a CombatActionVolume actor already provides the box.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Action Volume|Trace Box")
	bool bCreateTraceBox = true;

	/** Extent of the auto-created trace box */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Action Volume|Trace Box", meta=(EditCondition="bCreateTraceBox"))
	FVector TraceBoxExtent = FVector(100.0f, 100.0f, 100.0f);

	/** Relative offset of the auto-created trace box from the owner's root */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Action Volume|Trace Box", meta=(EditCondition="bCreateTraceBox"))
	FVector TraceBoxOffset = FVector::ZeroVector;

	// ~begin CombatUsable interface

	virtual bool CanUse_Implementation(AActor* Instigator) override;
	virtual FText GetUseText_Implementation(AActor* Instigator) override;
	virtual void Use_Implementation(AActor* Instigator) override;

	// ~end CombatUsable interface

	/** Returns the first action volume component on the actor, or null */
	UFUNCTION(BlueprintCallable, Category="Action Volume")
	static UCombatActionVolumeComponent* FindActionVolume(const AActor* Actor);

	virtual void OnRegister() override;
	virtual void OnUnregister() override;

protected:

	/** Auto-created trace box; only valid while registered and bCreateTraceBox is set */
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category="Action Volume|Trace Box")
	TObjectPtr<UBoxComponent> TraceBox;

	/** True after a single-use volume has been used */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Action Volume")
	bool bUsed = false;

	/** Creates the trace box on the owning actor */
	void CreateTraceBox();
};
