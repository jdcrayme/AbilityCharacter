// Copyright Epic Games, Inc. All Rights Reserved.

#include "WeaponEquipmentComponent.h"

#include "Features/Weapons/Weapon.h"
#include "Features/Weapons/WeaponData.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Core/AbilitySystem/CombatAbilitySystemComponent.h"
#include "Core/AbilitySystem/CombatGameplayTags.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Net/UnrealNetwork.h"

UWeaponEquipmentComponent::UWeaponEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

UWeaponEquipmentComponent* UWeaponEquipmentComponent::FindEquipmentOnActor(const AActor* Actor)
{
	// the component is optional by design: a civilian that was never given
	// weapon equipment simply has none, and every caller gets null instead of
	// a cast failure on a specific character class
	return Actor ? Actor->FindComponentByClass<UWeaponEquipmentComponent>() : nullptr;
}

AWeapon* UWeaponEquipmentComponent::GetEquippedWeaponOnActor(const AActor* Actor)
{
	const UWeaponEquipmentComponent* Equipment = FindEquipmentOnActor(Actor);
	return Equipment ? Equipment->GetEquippedWeapon() : nullptr;
}

UWeaponData* UWeaponEquipmentComponent::GetEquippedWeaponDataOnActor(const AActor* Actor)
{
	const UWeaponEquipmentComponent* Equipment = FindEquipmentOnActor(Actor);
	return Equipment ? Equipment->GetEquippedWeaponData() : nullptr;
}

bool UWeaponEquipmentComponent::EquipWeapon(UWeaponData* Data)
{
	if (!HasAuthority() || !Data || !Data->WeaponActorClass)
	{
		return false;
	}

	// equipping over an existing weapon simply replaces it (a proper swap
	// flow holsters first through the abilities; the component stays passive)
	if (EquippedWeapon)
	{
		UnequipWeapon();
	}

	ACharacter* Character = GetOwningCharacter();
	UWorld* World = GetWorld();
	if (!Character || !World)
	{
		return false;
	}

	// spawn the weapon and initialize it from its data (mesh, loaded magazine)
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Character;
	SpawnParams.Instigator = Character;
	AWeapon* Weapon = World->SpawnActor<AWeapon>(Data->WeaponActorClass, Character->GetActorTransform(), SpawnParams);
	if (!Weapon)
	{
		return false;
	}

	Weapon->InitWeapon(Data);

	// the weapon rides on the back until it is drawn
	Weapon->AttachToCharacter(GetOwningCharacterMesh(), BackSocketName);
	bWeaponDrawn = false;

	// the weapon's abilities are granted to the character's ability system
	// for as long as it stays equipped - the same grant/revoke pair the
	// character's own defaults use
	GrantWeaponAbilities(Data);

	// Equipped tags go on through the replicated State.Weapon slot, exactly
	// like stance tags go through State.Stance
	ApplyWeaponStateTags(false);

	AWeapon* OldWeapon = EquippedWeapon;
	EquippedWeapon = Weapon;
	OnEquippedWeaponChanged.Broadcast(OldWeapon, Weapon);

	return true;
}

void UWeaponEquipmentComponent::DrawWeapon()
{
	if (!HasAuthority() || !EquippedWeapon)
	{
		return;
	}

	// attach to the hand and flip the drawn flag; the anim layer link is part
	// of the cosmetic state every machine applies
	bWeaponDrawn = true;
	EquippedWeapon->AttachToCharacter(GetOwningCharacterMesh(), HandSocketName);
	ApplyCosmeticState(GetOwningCharacter());

	// the slot replace swaps Equipped-only tags for Equipped + Drawn
	ApplyWeaponStateTags(true);
}

void UWeaponEquipmentComponent::HolsterWeapon()
{
	if (!HasAuthority() || !EquippedWeapon)
	{
		return;
	}

	bWeaponDrawn = false;
	EquippedWeapon->AttachToCharacter(GetOwningCharacterMesh(), BackSocketName);
	ApplyCosmeticState(GetOwningCharacter());

	ApplyWeaponStateTags(false);
}

void UWeaponEquipmentComponent::UnequipWeapon()
{
	if (!HasAuthority() || !EquippedWeapon)
	{
		return;
	}

	RevokeWeaponAbilities();

	// clear the whole slot: no weapon equipped, no weapon tags
	if (UCombatAbilitySystemComponent* CombatASC = Cast<UCombatAbilitySystemComponent>(GetOwningASC()))
	{
		CombatASC->ClearGrantedTags(TAG_State_Weapon);
	}

	bWeaponDrawn = false;

	AWeapon* OldWeapon = EquippedWeapon;
	EquippedWeapon = nullptr;
	OnEquippedWeaponChanged.Broadcast(OldWeapon, nullptr);

	// the magazine travels with the actor: whatever a dropped weapon keeps,
	// it keeps because its state component keeps it
	OldWeapon->Destroy();
}

AWeapon* UWeaponEquipmentComponent::GetEquippedWeapon() const
{
	return EquippedWeapon;
}

UWeaponData* UWeaponEquipmentComponent::GetEquippedWeaponData() const
{
	return EquippedWeapon ? EquippedWeapon->WeaponData : nullptr;
}

bool UWeaponEquipmentComponent::IsWeaponDrawn() const
{
	return bWeaponDrawn && EquippedWeapon != nullptr;
}

void UWeaponEquipmentComponent::ApplyWeaponStateTags(bool bDrawn)
{
	UCombatAbilitySystemComponent* CombatASC = Cast<UCombatAbilitySystemComponent>(GetOwningASC());
	if (!CombatASC)
	{
		return;
	}

	// effect-granted tags replicate to every client, which is what animation
	// blueprints and gated abilities on simulated proxies need; replacing the
	// slot's tags removes whatever the previous state granted
	FGameplayTagContainer StateTags;
	StateTags.AddTag(TAG_State_Weapon_Equipped);
	if (bDrawn)
	{
		StateTags.AddTag(TAG_State_Weapon_Drawn);
	}

	CombatASC->ApplyGrantedTags(TAG_State_Weapon, StateTags);
}

void UWeaponEquipmentComponent::GrantWeaponAbilities(const UWeaponData* Data)
{
	UAbilitySystemComponent* ASC = GetOwningASC();
	if (!ASC || !Data)
	{
		return;
	}

	for (const FWeaponAbilityGrant& Grant : Data->GrantedAbilities)
	{
		if (Grant.AbilityClass)
		{
			const FGameplayAbilitySpec Spec(Grant.AbilityClass, Grant.AbilityLevel, INDEX_NONE, GetOwner());
			const FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
			GrantedAbilityHandles.Add(Handle);
		}
	}
}

void UWeaponEquipmentComponent::RevokeWeaponAbilities()
{
	UAbilitySystemComponent* ASC = GetOwningASC();
	if (!ASC)
	{
		return;
	}

	for (const FGameplayAbilitySpecHandle& Handle : GrantedAbilityHandles)
	{
		ASC->ClearAbility(Handle);
	}

	GrantedAbilityHandles.Reset();
}

void UWeaponEquipmentComponent::ApplyCosmeticState(ACharacter* Character)
{
	// attach point and overlay anim layer are cosmetic: the server sets them
	// authoritatively, clients re-apply them from the OnReps so simulated
	// proxies hold the weapon correctly without running any ability logic
	if (!Character || !EquippedWeapon || !EquippedWeapon->WeaponData)
	{
		return;
	}

	USkeletalMeshComponent* CharacterMesh = Character->GetMesh();
	if (!CharacterMesh)
	{
		return;
	}

	// the overlay anim layer (AL_Weapon_Rifle-style) implements the
	// character's layer interface; linking it is what keeps the character's
	// anim blueprint weapon-agnostic while the weapon owns its upper-body
	// pose, mask depth and aim offsets
	TSubclassOf<UAnimInstance> LayerClass = EquippedWeapon->WeaponData->OverlayAnimLayerClass;
	if (bWeaponDrawn && LayerClass)
	{
		CharacterMesh->LinkAnimClassLayers(LayerClass);
	}
	else if (LayerClass)
	{
		CharacterMesh->UnlinkAnimClassLayers(LayerClass);
	}
}

void UWeaponEquipmentComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// the weapon actor dies with its owner; no orphaned actors on server shutdown
	if (EquippedWeapon && HasAuthority())
	{
		EquippedWeapon->Destroy();
	}

	Super::EndPlay(EndPlayReason);
}

void UWeaponEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWeaponEquipmentComponent, EquippedWeapon);
	DOREPLIFETIME(UWeaponEquipmentComponent, bWeaponDrawn);
}

ACharacter* UWeaponEquipmentComponent::GetOwningCharacter() const
{
	return Cast<ACharacter>(GetOwner());
}

USkeletalMeshComponent* UWeaponEquipmentComponent::GetOwningCharacterMesh() const
{
	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	return Character ? Character->GetMesh() : nullptr;
}

UAbilitySystemComponent* UWeaponEquipmentComponent::GetOwningASC() const
{
	const AActor* Owner = GetOwner();
	const IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Owner);
	return ASCInterface ? ASCInterface->GetAbilitySystemComponent() : nullptr;
}

void UWeaponEquipmentComponent::OnRep_EquippedWeapon(AWeapon* OldWeapon)
{
	// re-apply the attach point locally; the layer link follows the drawn flag
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachToCharacter(GetOwningCharacterMesh(), bWeaponDrawn ? HandSocketName : BackSocketName);
		ApplyCosmeticState(GetOwningCharacter());
	}

	OnEquippedWeaponChanged.Broadcast(OldWeapon, EquippedWeapon);
}

void UWeaponEquipmentComponent::OnRep_IsWeaponDrawn()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachToCharacter(GetOwningCharacterMesh(), bWeaponDrawn ? HandSocketName : BackSocketName);
		ApplyCosmeticState(GetOwningCharacter());
	}
}

bool UWeaponEquipmentComponent::HasAuthority() const
{
	const AActor* Owner = GetOwner();
	return Owner && Owner->HasAuthority();
}
