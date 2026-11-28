// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "WeaponEquipmentComponent.generated.h"

class AWeapon;
class UWeaponData;
class UGameplayAbility;
class UAbilitySystemComponent;
class USkeletalMeshComponent;
class ACharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquippedWeaponChanged, AWeapon*, OldWeapon, AWeapon*, NewWeapon);

/**
 *  The modularity seam between a character and the weapon feature.
 *
 *  Responsibilities, and nothing else:
 *  1. Spawning and attaching the weapon actor (back socket when equipped,
 *     hand socket when drawn) from a UWeaponData asset.
 *  2. Granting the weapon's abilities to the owner's ability system while
 *     equipped, and revoking them on unequip - through the exact same
 *     GiveAbility/ClearAbility pair CharacterBase::GrantAbility uses.
 *  3. Maintaining the replicated State.Weapon tag slot (Equipped, Drawn) so
 *     ability gating, animation blueprints and UI all key off the same tags.
 *  4. Linking the weapon's overlay anim layer onto the character's mesh while
 *     drawn and unlinking it when holstered.
 *
 *  The component is deliberately passive: it never plays montages and never
 *  decides when anything happens. The weapon abilities (equip, fire, reload,
 *  aim) drive it; AI, UI and Blueprints reach it through FindEquipmentOnActor
 *  or the character's own component list, so no ability ever casts to a
 *  specific character class.
 */
UCLASS(ClassGroup = (Equipment), meta = (BlueprintSpawnableComponent))
class ABILITYCHARACTER_API UWeaponEquipmentComponent : public UActorComponent
{
        GENERATED_BODY()

public:

        UWeaponEquipmentComponent();

        /** Returns the weapon equipment component on an actor (weapon abilities resolve their equipment through this) */
        UFUNCTION(BlueprintPure, Category = "Equipment")
        static UWeaponEquipmentComponent* FindEquipmentOnActor(const AActor* Actor);

        /** Returns the equipped weapon of an actor through its equipment component, or null */
        UFUNCTION(BlueprintPure, Category = "Equipment")
        static AWeapon* GetEquippedWeaponOnActor(const AActor* Actor);

        /** Returns the weapon data of the equipped weapon of an actor, or null */
        UFUNCTION(BlueprintPure, Category = "Equipment")
        static UWeaponData* GetEquippedWeaponDataOnActor(const AActor* Actor);

        /** Broadcasts whenever the equipped weapon changes (all machines; UI and AI listen here) */
        UPROPERTY(BlueprintAssignable, Category = "Equipment")
        FOnEquippedWeaponChanged OnEquippedWeaponChanged;

        /**
         *  Equips a weapon from its data asset: spawns the actor, attaches it to
         *  the back socket, grants the weapon's abilities and applies the
         *  State.Weapon slot tags. Does not draw the weapon and plays no animation;
         *  the equip ability owns that flow. Server only.
         */
        UFUNCTION(BlueprintCallable, Category = "Equipment")
        bool EquipWeapon(UWeaponData* Data);

        /**
         *  Draws the weapon: attaches it to the hand socket, links the weapon's
         *  overlay anim layer onto the character's mesh and applies the
         *  State.Weapon.Drawn tags. Called by the equip ability at its swap notify;
         *  also safe to call directly for instant (montageless) draws. Server only.
         */
        UFUNCTION(BlueprintCallable, Category = "Equipment")
        void DrawWeapon();

        /**
         *  Holsters the weapon: attaches it back to the back socket, unlinks the
         *  overlay anim layer and drops the Drawn tags. Server only.
         */
        UFUNCTION(BlueprintCallable, Category = "Equipment")
        void HolsterWeapon();

        /**
         *  Unequips the weapon: revokes the granted abilities, clears the
         *  State.Weapon slot and destroys the weapon actor. The magazine travels
         *  with the actor; a future drop/pickup flow reuses exactly this seam.
         *  Server only.
         */
        UFUNCTION(BlueprintCallable, Category = "Equipment")
        void UnequipWeapon();

        UFUNCTION(BlueprintPure, Category = "Equipment")
        AWeapon* GetEquippedWeapon() const;

        UFUNCTION(BlueprintPure, Category = "Equipment")
        UWeaponData* GetEquippedWeaponData() const;

        UFUNCTION(BlueprintPure, Category = "Equipment")
        bool IsWeaponDrawn() const;

        /** Socket on the character's mesh weapons attach to when drawn */
        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
        FName HandSocketName = TEXT("hand_r");

        /** Socket on the character's mesh weapons attach to when holstered */
        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
        FName BackSocketName = TEXT("spine_03");

protected:

        virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
        virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:

        /** The equipped weapon actor; server assigns, clients receive */
        UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
        TObjectPtr<AWeapon> EquippedWeapon;

        /** Whether the equipped weapon is currently drawn; drives cosmetic re-application on clients */
        UPROPERTY(ReplicatedUsing = OnRep_IsWeaponDrawn)
        bool bWeaponDrawn = false;

        /** Handles of the abilities granted by the equipped weapon, revoked on unequip */
        UPROPERTY()
        TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;

        /** Applies the State.Weapon slot tags for the current equipment state */
        void ApplyWeaponStateTags(bool bDrawn);

        /** Grants or revokes the data's ability set */
        void GrantWeaponAbilities(const UWeaponData* Data);
        void RevokeWeaponAbilities();

        /** Cosmetic state application that every machine runs: attach point and anim layer */
        void ApplyCosmeticState(ACharacter* Character);

        /** Returns the owning character, or null */
        ACharacter* GetOwningCharacter() const;

        /** Returns the owning character's mesh, or null */
        USkeletalMeshComponent* GetOwningCharacterMesh() const;

        /** Returns the owner's ability system component through IAbilitySystemInterface, or null */
        UAbilitySystemComponent* GetOwningASC() const;

        UFUNCTION()
        void OnRep_EquippedWeapon(AWeapon* OldWeapon);

        UFUNCTION()
        void OnRep_IsWeaponDrawn();

        bool HasAuthority() const;
};
