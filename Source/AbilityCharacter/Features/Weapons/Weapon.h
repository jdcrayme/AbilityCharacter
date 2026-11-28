// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class USkeletalMeshComponent;
class UWeaponData;
class UWeaponStateComponent;

/**
 *  Deliberately dumb weapon actor: mesh, sockets and its magazine state.
 *  All behaviour (fire, reload, aim, equip) lives in gameplay abilities
 *  granted by the equipment component; all numbers live in UWeaponData.
 *  If a new weapon needs code, it should need a new ability or new data,
 *  not a new actor subclass.
 *
 *  Sockets the content contract expects on the mesh:
 *  - muzzle (configurable per weapon through FWeaponFiringData): shots and
 *    tracer effects originate here
 *  - GripLeft: the left hand IK target when two-handing the weapon
 */
UCLASS()
class ABILITYCHARACTER_API AWeapon : public AActor
{
	GENERATED_BODY()

public:

	AWeapon();

	/** Weapon mesh: visuals and sockets. No collision, no physics, no input */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USkeletalMeshComponent> Mesh;

	/** Magazine state: replicated mag count, size, change delegate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UWeaponStateComponent> StateComponent;

	/** Data asset this weapon was spawned from; replicated so clients can read sockets, montages and ammo types */
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UWeaponData> WeaponData;

	/**
	 *  Initializes the weapon from its data (mesh, magazine). Called by the
	 *  equipment component right after spawning; server only.
	 */
	void InitWeapon(UWeaponData* Data);

	/** Transform of a socket on the weapon mesh (identity when the socket is missing) */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	FTransform GetSocketTransformByName(FName SocketName) const;

	/** Transform of the muzzle socket shots and tracer effects originate from */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	FTransform GetMuzzleTransform() const;

	/** Transform of the left hand IK target for two-handed grips */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	FTransform GetLeftHandGripTransform() const;

	/**
	 *  Attaches the weapon to a socket on the character's mesh. The equipment
	 *  component calls this on the server when equipping/drawing/holstering;
	 *  the replicated attachment (and the OnReps on the equipment component)
	 *  carry it to the clients.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void AttachToCharacter(USkeletalMeshComponent* CharacterMesh, FName SocketName);

protected:

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Socket the left hand IK reads from */
	static FName LeftHandGripSocketName;
};
