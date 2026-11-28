// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "WeaponData.generated.h"

class AWeapon;
class UAnimMontage;
class UAnimInstance;
class UGameplayEffect;
class UGameplayAbility;
class USkeletalMesh;

/**
 *  One granted weapon ability: granted to the owner's ability system while the
 *  weapon is equipped, revoked when it is unequipped. The tag is the ability's
 *  identity (Ability.Weapon.Fire and so on), so input bindings, Blueprints and
 *  AI activate granted weapon abilities exactly like stance abilities.
 */
USTRUCT(BlueprintType)
struct FWeaponAbilityGrant
{
	GENERATED_BODY()

	/** Ability class granted while this weapon is equipped */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	TSubclassOf<UGameplayAbility> AbilityClass;

	/** Tag that identifies the ability once granted (its asset tag) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	FGameplayTag AbilityTag;

	/** Level the ability is granted at */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	int32 AbilityLevel = 1;
};

/**
 *  Firing parameters for hitscan weapons. Everything here flows into the fire
 *  ability's trace and damage through SetByCaller magnitudes and effect context,
 *  so one UWeaponFireAbility class serves every hitscan weapon ever authored.
 */
USTRUCT(BlueprintType)
struct FWeaponFiringData
{
	GENERATED_BODY()

	/** Maximum distance of the hitscan trace */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing", meta = (ClampMin = 0, Units = "cm"))
	float TraceDistance = 5000.0f;

	/** Radius of the sphere used by the hitscan trace */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing", meta = (ClampMin = 0, ClampMax = 200, Units = "cm"))
	float TraceRadius = 10.0f;

	/** Damage dealt per hit, passed through the Data.Damage SetByCaller magnitude */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing", meta = (ClampMin = 0))
	float BaseDamage = 20.0f;

	/** Knockback impulse applied to a hit target */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm/s"))
	float KnockbackImpulse = 100.0f;

	/** Upwards launch impulse applied to a hit target */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm/s"))
	float LaunchImpulse = 50.0f;

	/** Seconds between shots; used as the fire ability's cooldown duration through the Data.Cooldown SetByCaller magnitude */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float FireInterval = 0.15f;

	/** Socket on the weapon mesh that shots and effects originate from */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing")
	FName MuzzleSocketName = TEXT("muzzle");

	/** Whether shots also hit world-dynamic objects such as destructibles */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing")
	bool bDamageWorldObjects = true;

	/**
	 *  Damage effect applied to targets (a GE_DamageHealth-style effect using the
	 *  Data.Damage SetByCaller). Null falls back to the fire ability's default.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing")
	TSubclassOf<UGameplayEffect> DamageEffect;

	/**
	 *  Cooldown effect applied per shot (duration magnitude set by the fire
	 *  ability through the Data.Cooldown SetByCaller, so the same GE asset works
	 *  for every weapon). Null means no cooldown; the fire montage's own length
	 *  then paces semi-auto fire.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing")
	TSubclassOf<UGameplayEffect> CooldownEffect;
};

/**
 *  Magazine and ammunition parameters. The ammo type tag is the type system:
 *  the weapon consumes Ammo.X, inventory entries count Ammo.X, and reload
 *  moves rounds from the inventory's reserve entry into the weapon's magazine.
 */
USTRUCT(BlueprintType)
struct FWeaponAmmoData
{
	GENERATED_BODY()

	/** Ammunition type this weapon consumes (Ammo.Rifle, Ammo.Shotgun...) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	FGameplayTag AmmoTypeTag;

	/** Rounds the magazine holds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = 0))
	int32 MagSize = 30;

	/** Rounds loaded into the magazine when the weapon is created */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = 0))
	int32 StartingMagCount = 30;
};

/**
 *  Data-only description of a weapon: the actor class to spawn, the visual
 *  mesh, the abilities granted while equipped, the montages the abilities
 *  play, the firing and ammunition numbers. New weapons are new instances of
 *  this asset - never new C++ classes.
 *
 *  The equip/reload montages carry AnimNotify_SendGameplayEvent instances:
 *  Event.Weapon.Swap in equip montages (the frame the hand reaches the weapon,
 *  when the equip ability attaches it and links the overlay anim layer) and
 *  Event.Weapon.MagIn in reload montages (the frame the magazine seats, when
 *  the reload ability moves rounds from the inventory reserve into the
 *  magazine). Fire montages use the existing attack trace notify, exactly
 *  like ranged attack montages do.
 */
UCLASS(BlueprintType)
class ABILITYCHARACTER_API UWeaponData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	/** Actor class spawned by the equipment component when this weapon is equipped */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AWeapon> WeaponActorClass;

	/** Mesh set on the weapon actor; the actor owns sockets (muzzle, grip...), this owns the visuals */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USkeletalMesh> Mesh;

	/**
	 *  Anim layer class linked onto the character's mesh while this weapon is
	 *  drawn (an AL_Weapon_Rifle-style linked anim layer implementing the
	 *  character's overlay interface). Null keeps the character's own layers.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TSubclassOf<UAnimInstance> OverlayAnimLayerClass;

	/** Abilities granted while equipped and revoked on unequip */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
	TArray<FWeaponAbilityGrant> GrantedAbilities;

	/** Montage played by the equip ability when the weapon is drawn */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> EquipMontage;

	/** Montage played when the weapon is holstered */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> UnequipMontage;

	/** Montage played by the reload ability */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> ReloadMontage;

	/** Montage played by the fire ability; its attack trace notify is the fire frame */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> FireMontage;

	/** Firing parameters consumed by the fire ability at runtime */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing")
	FWeaponFiringData Firing;

	/** Magazine and ammunition parameters */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	FWeaponAmmoData Ammo;
};
