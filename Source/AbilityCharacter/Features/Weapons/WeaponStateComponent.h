// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponStateComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponAmmoChanged, int32, MagCount, int32, MagSize);

/**
 *  The weapon's operational state: the rounds currently loaded into the
 *  magazine. This is deliberately weapon-local (and therefore travels with
 *  the weapon actor when it is dropped or handed over) rather than living in
 *  the character's inventory: the inventory is the possession layer (what
 *  the character carries - the reserve rounds), while this component is the
 *  weapon's operational layer (what the weapon is doing right now).
 *
 *  The fire ability consumes from it at the exact fire-frame notify, and the
 *  reload ability refills it from the inventory's reserve at the magazine-in
 *  notify. Ammo counts replicate as one integer with an OnRep, which is all
 *  the HUD needs; when multiple weapons with per-client ammo change batching
 *  become a concern, upgrade this to a FastArraySerializer like Lyra does.
 */
UCLASS(ClassGroup = (Weapon), meta = (BlueprintSpawnableComponent))
class ABILITYCHARACTER_API UWeaponStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UWeaponStateComponent();

	/** Broadcast whenever the magazine count changes (all machines; UI listens here) */
	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnWeaponAmmoChanged OnAmmoChanged;

	/** Rounds the magazine can hold; initialized from the weapon data */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon")
	int32 MagSize = 0;

	/** Rounds currently loaded */
	UPROPERTY(ReplicatedUsing = OnRep_MagCount, BlueprintReadOnly, Category = "Weapon")
	int32 MagCount = 0;

	/** Initializes the magazine from the weapon data; server only */
	void InitState(int32 InMagSize, int32 InMagCount);

	/** Rounds still fitting into the magazine */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetMagRoom() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsMagEmpty() const;

	/**
	 *  Consumes rounds from the magazine. Returns whether the rounds were
	 *  actually available and consumed. Server only (a predicted client
	 *  instance just asks and trusts the replication for the HUD).
	 */
	bool ConsumeAmmo(int32 Amount);

	/**
	 *  Loads rounds into the magazine (capped by MagSize) and returns how many
	 *  actually fit. Server only; the reload ability uses the return value to
	 *  give the unused rounds back to the inventory reserve.
	 */
	int32 RefillMag(int32 Amount);

protected:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Broadcasts the change on clients; the server broadcast happens in the mutators */
	UFUNCTION()
	void OnRep_MagCount(int32 OldMagCount);

private:

	/** Server-side guard for the mutators */
	bool HasAuthority() const;

	/** Server-side change broadcast so the owning side hears it immediately too */
	void BroadcastChange();
};
