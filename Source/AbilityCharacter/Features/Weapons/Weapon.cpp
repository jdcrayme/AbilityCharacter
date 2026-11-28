// Copyright Epic Games, Inc. All Rights Reserved.

#include "Weapon.h"
#include "WeaponData.h"
#include "WeaponStateComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"

FName AWeapon::LeftHandGripSocketName(TEXT("GripLeft"));

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	// the equipment component owns the weapon's lifetime; attached actors have
	// no use for movement replication
	bReplicates = true;
	SetReplicateMovement(false);

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	// the character sweeps for hits; the weapon itself never blocks them
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	StateComponent = CreateDefaultSubobject<UWeaponStateComponent>(TEXT("StateComponent"));
}

void AWeapon::InitWeapon(UWeaponData* Data)
{
	WeaponData = Data;

	if (Data)
	{
		if (Data->Mesh)
		{
			Mesh->SetSkeletalMesh(Data->Mesh);
		}

		// the magazine starts loaded; the reserve stays behind in the inventory
		StateComponent->InitState(Data->Ammo.MagSize, Data->Ammo.StartingMagCount);
	}
}

FTransform AWeapon::GetSocketTransformByName(FName SocketName) const
{
	if (Mesh && SocketName != NAME_None && Mesh->DoesSocketExist(SocketName))
	{
		return Mesh->GetSocketTransform(SocketName);
	}

	return FTransform::Identity;
}

FTransform AWeapon::GetMuzzleTransform() const
{
	// the socket name comes from the weapon data so each weapon can name its own
	const FName Socket = (WeaponData ? WeaponData->Firing.MuzzleSocketName : FName(TEXT("muzzle")));
	return GetSocketTransformByName(Socket);
}

FTransform AWeapon::GetLeftHandGripTransform() const
{
	return GetSocketTransformByName(LeftHandGripSocketName);
}

void AWeapon::AttachToCharacter(USkeletalMeshComponent* CharacterMesh, FName SocketName)
{
	if (CharacterMesh && Mesh)
	{
		AttachToComponent(CharacterMesh, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), SocketName);
	}
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponData);
}
