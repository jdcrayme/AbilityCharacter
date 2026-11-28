// Copyright Epic Games, Inc. All Rights Reserved.

#include "CombatInventoryComponent.h"

#include "Net/UnrealNetwork.h"

UCombatInventoryComponent::UCombatInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UCombatInventoryComponent::AddEntry(FGameplayTag TypeTag, int32 Count)
{
	if (!HasAuthority() || !TypeTag.IsValid() || Count <= 0)
	{
		return;
	}

	if (FCombatInventoryEntry* Entry = FindEntry(TypeTag))
	{
		Entry->Count += Count;
		OnEntryChanged.Broadcast(TypeTag, Entry->Count);
	}
	else
	{
		FCombatInventoryEntry NewEntry;
		NewEntry.TypeTag = TypeTag;
		NewEntry.Count = Count;
		const int32 NewIndex = Entries.Add(NewEntry);
		OnEntryChanged.Broadcast(TypeTag, Entries[NewIndex].Count);
	}
}

bool UCombatInventoryComponent::RemoveEntries(FGameplayTag TypeTag, int32 Count)
{
	if (!HasAuthority() || !TypeTag.IsValid() || Count <= 0)
	{
		return false;
	}

	FCombatInventoryEntry* Entry = FindEntry(TypeTag);
	if (!Entry || Entry->Count < Count)
	{
		return false;
	}

	Entry->Count -= Count;
	OnEntryChanged.Broadcast(TypeTag, Entry->Count);

	// keep empty entries out of the array: an entry with zero is a type the
	// character no longer carries
	if (Entry->Count == 0)
	{
		Entries.RemoveAll([TypeTag](const FCombatInventoryEntry& Candidate)
		{
			return Candidate.TypeTag == TypeTag;
		});
	}

	return true;
}

int32 UCombatInventoryComponent::GetEntryCount(FGameplayTag TypeTag) const
{
	for (const FCombatInventoryEntry& Entry : Entries)
	{
		if (Entry.TypeTag == TypeTag)
		{
			return Entry.Count;
		}
	}

	return 0;
}

void UCombatInventoryComponent::ClearInventory()
{
	if (!HasAuthority())
	{
		return;
	}

	Entries.Reset();
}

void UCombatInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatInventoryComponent, Entries);
}

FCombatInventoryEntry* UCombatInventoryComponent::FindEntry(const FGameplayTag& TypeTag)
{
	for (FCombatInventoryEntry& Entry : Entries)
	{
		if (Entry.TypeTag == TypeTag)
		{
			return &Entry;
		}
	}

	return nullptr;
}

void UCombatInventoryComponent::OnRep_Entries()
{
	// the array arrived wholesale; fire the generic change notification so UI
	// re-reads whatever entries it displays
	OnEntryChanged.Broadcast(FGameplayTag(), -1);
}

bool UCombatInventoryComponent::HasAuthority() const
{
	const AActor* Owner = GetOwner();
	return Owner && Owner->HasAuthority();
}
