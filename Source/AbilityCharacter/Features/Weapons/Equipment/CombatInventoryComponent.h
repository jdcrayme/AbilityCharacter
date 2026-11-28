// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "CombatInventoryComponent.generated.h"

/**
 *  One kind of thing the character possesses, keyed by a gameplay tag:
 *  Ammo.Rifle x 90, Medkit x 3, Keycard.Blue x 1... The tag is the type
 *  system, so new item types need no new classes - and ammo entries use the
 *  same tags the weapons consume, which is what makes the reload transfer
 *  a plain tag-keyed move between two pools.
 */
USTRUCT(BlueprintType)
struct FCombatInventoryEntry
{
	GENERATED_BODY()

	/** Type of the counted thing (Ammo.Rifle, Medkit...) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	FGameplayTag TypeTag;

	/** How many of it are possessed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 Count = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryEntryChanged, FGameplayTag, TypeTag, int32, NewCount);

/**
 *  Generic possession layer: what the character carries, in what quantities,
 *  keyed by gameplay tags. Deliberately knows nothing about weapons, magazines
 *  or reloads - the weapon-specific side of ammunition (the loaded magazine)
 *  lives on the weapon actor's UWeaponStateComponent instead, and reload is a
 *  transfer between the two: inventory.Remove(Ammo.X, N) then weapon's state
 *  refilling by N.
 *
 *  This split is what answers "dropped with the weapon": the magazine belongs
 *  to the weapon actor and therefore travels with it automatically, while the
 *  backpack's reserve rounds stay with the character. Keeping both pools in
 *  one component would force a transfer transaction on every drop and pickup
 *  for a number the weapon already owns.
 *
 *  Mutations are server-authoritative. Replication is a plain array with an
 *  OnRep for now; when entry counts grow, upgrade to a FastArraySerializer
 *  (Lyra's inventory manager) for per-entry delta replication.
 */
UCLASS(ClassGroup = (Equipment), meta = (BlueprintSpawnableComponent))
class ABILITYCHARACTER_API UCombatInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UCombatInventoryComponent();

	/** Broadcasts whenever an entry's count changes (all machines; UI listens here) */
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryEntryChanged OnEntryChanged;

	/** Adds Count to (or creates) the entry for TypeTag; server only */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddEntry(FGameplayTag TypeTag, int32 Count);

	/**
	 *  Removes Count from the entry for TypeTag; fails (and removes nothing)
	 *  when the character doesn't carry enough. Server only.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveEntries(FGameplayTag TypeTag, int32 Count);

	/** How many of TypeTag the character carries */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetEntryCount(FGameplayTag TypeTag) const;

	/** Drops every entry; server only */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ClearInventory();

protected:

	/** Possessed entries; server mutates, clients receive the array wholesale */
	UPROPERTY(ReplicatedUsing = OnRep_Entries, BlueprintReadOnly, Category = "Inventory")
	TArray<FCombatInventoryEntry> Entries;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:

	/** Points into Entries for a tag (or null when the character carries none) */
	FCombatInventoryEntry* FindEntry(const FGameplayTag& TypeTag);

	UFUNCTION()
	void OnRep_Entries();

	bool HasAuthority() const;
};
