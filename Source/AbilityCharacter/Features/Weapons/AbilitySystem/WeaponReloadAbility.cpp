// Copyright Epic Games, Inc. All Rights Reserved.

#include "WeaponReloadAbility.h"
#include "AbilityCharacter.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "Core/AbilitySystem/CombatGameplayTags.h"
#include "Features/Weapons/Equipment/WeaponEquipmentComponent.h"
#include "Features/Weapons/Equipment/CombatInventoryComponent.h"
#include "Features/Weapons/Weapon.h"
#include "Features/Weapons/WeaponData.h"
#include "Features/Weapons/WeaponStateComponent.h"

UWeaponReloadAbility::UWeaponReloadAbility()
{
	// reload identity: input bindings, Blueprints and AI activate through
	// Ability.Weapon.Reload
	SetAssetTags(FGameplayTagContainer(TAG_Ability_Weapon_Reload));

	// the ability activates from the reload input press event
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = TAG_Event_InputReloadPressed;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

	// only a drawn weapon reloads, and a swap can't start mid-reload
	ActivationRequiredTags.AddTag(TAG_State_Weapon_Drawn);
	ActivationBlockedTags.AddTag(TAG_State_Weapon_Swapping);
}

bool UWeaponReloadAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const AActor* Avatar = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	const AWeapon* Weapon = UWeaponEquipmentComponent::GetEquippedWeaponOnActor(Avatar);
	const UWeaponData* WeaponData = UWeaponEquipmentComponent::GetEquippedWeaponDataOnActor(Avatar);
	if (!Weapon || !Weapon->StateComponent || !WeaponData)
	{
		return false;
	}

	// nothing to reload: magazine full or nothing in the reserve
	if (Weapon->StateComponent->GetMagRoom() <= 0)
	{
		return false;
	}

	const UCombatInventoryComponent* Inventory = Avatar ? Avatar->FindComponentByClass<UCombatInventoryComponent>() : nullptr;
	if (!Inventory)
	{
		return false;
	}

	return Inventory->GetEntryCount(WeaponData->Ammo.AmmoTypeTag) > 0;
}

void UWeaponReloadAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	const UWeaponData* WeaponData = UWeaponEquipmentComponent::GetEquippedWeaponDataOnActor(GetAvatarActorFromActorInfo());
	if (!WeaponData || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// blocking state for the reload's duration: fire and equip gate on it
	FGameplayTagContainer ReloadingTags;
	ReloadingTags.AddTag(TAG_State_Weapon_Reloading);
	ApplyStateSlot(TAG_State_Weapon_Reloading, ReloadingTags);

	if (WeaponData->ReloadMontage)
	{
		MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, WeaponData->ReloadMontage);
		MontageTask->OnCompleted.AddDynamic(this, &UWeaponReloadAbility::OnMontageFinished);
		MontageTask->OnInterrupted.AddDynamic(this, &UWeaponReloadAbility::OnMontageFinished);
		MontageTask->OnCancelled.AddDynamic(this, &UWeaponReloadAbility::OnMontageFinished);

		// the magazine-in notify is the transfer frame
		MagInTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TAG_Event_Weapon_MagIn);
		MagInTask->EventReceived.AddDynamic(this, &UWeaponReloadAbility::OnMagInEvent);

		MontageTask->ReadyForActivation();
		MagInTask->ReadyForActivation();
	}
	else
	{
		// montageless weapons reload instantly (turrets, test cubes)
		TransferAmmo();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UWeaponReloadAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// clean the blocking state whether the reload completed, was interrupted
	// by damage or was cancelled outright
	ClearStateSlot(TAG_State_Weapon_Reloading);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UWeaponReloadAbility::OnMagInEvent(FGameplayEventData EventData)
{
	TransferAmmo();
}

void UWeaponReloadAbility::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UWeaponReloadAbility::TransferAmmo()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	AWeapon* Weapon = UWeaponEquipmentComponent::GetEquippedWeaponOnActor(Avatar);
	UWeaponData* WeaponData = UWeaponEquipmentComponent::GetEquippedWeaponDataOnActor(Avatar);

	if (!Avatar || !Weapon || !WeaponData || !Weapon->StateComponent)
	{
		return;
	}

	// the reserve is the inventory's, the magazine the weapon's: the server's
	// instance of this ability is the only one allowed to move rounds across
	if (!Avatar->HasAuthority())
	{
		return;
	}

	UCombatInventoryComponent* Inventory = Avatar->FindComponentByClass<UCombatInventoryComponent>();
	if (!Inventory)
	{
		return;
	}

	// move as many rounds as the magazine has room for and the reserve holds
	const int32 Room = Weapon->StateComponent->GetMagRoom();
	const int32 Available = Inventory->GetEntryCount(WeaponData->Ammo.AmmoTypeTag);
	const int32 ToMove = FMath::Min(Room, Available);

	if (ToMove > 0 && Inventory->RemoveEntries(WeaponData->Ammo.AmmoTypeTag, ToMove))
	{
		// RefillMag caps at the magazine's room and returns what actually
		// seated; give any excess back to the reserve (shouldn't happen, but
		// the transfer must never lose rounds)
		const int32 Seated = Weapon->StateComponent->RefillMag(ToMove);
		const int32 Excess = ToMove - Seated;
		if (Excess > 0)
		{
			Inventory->AddEntry(WeaponData->Ammo.AmmoTypeTag, Excess);
		}
	}
}
