// Copyright Epic Games, Inc. All Rights Reserved.

#include "WeaponStateComponent.h"

#include "Net/UnrealNetwork.h"

UWeaponStateComponent::UWeaponStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UWeaponStateComponent::InitState(int32 InMagSize, int32 InMagCount)
{
	if (!HasAuthority())
	{
		return;
	}

	MagSize = FMath::Max(0, InMagSize);
	MagCount = FMath::Clamp(InMagCount, 0, MagSize);

	BroadcastChange();
}

int32 UWeaponStateComponent::GetMagRoom() const
{
	return FMath::Max(0, MagSize - MagCount);
}

bool UWeaponStateComponent::IsMagEmpty() const
{
	return MagCount <= 0;
}

bool UWeaponStateComponent::ConsumeAmmo(int32 Amount)
{
	if (!HasAuthority() || Amount <= 0)
	{
		return false;
	}

	if (MagCount < Amount)
	{
		return false;
	}

	MagCount -= Amount;
	BroadcastChange();
	return true;
}

int32 UWeaponStateComponent::RefillMag(int32 Amount)
{
	if (!HasAuthority() || Amount <= 0)
	{
		return 0;
	}

	const int32 Room = GetMagRoom();
	const int32 Loaded = FMath::Min(Room, Amount);

	if (Loaded > 0)
	{
		MagCount += Loaded;
		BroadcastChange();
	}

	return Loaded;
}

void UWeaponStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWeaponStateComponent, MagSize);
	DOREPLIFETIME(UWeaponStateComponent, MagCount);
}

void UWeaponStateComponent::OnRep_MagCount(int32 OldMagCount)
{
	OnAmmoChanged.Broadcast(MagCount, MagSize);
}

bool UWeaponStateComponent::HasAuthority() const
{
	const AActor* Owner = GetOwner();
	return Owner && Owner->HasAuthority();
}

void UWeaponStateComponent::BroadcastChange()
{
	OnAmmoChanged.Broadcast(MagCount, MagSize);
}
