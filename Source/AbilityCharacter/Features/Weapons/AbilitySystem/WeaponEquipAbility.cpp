// Copyright Epic Games, Inc. All Rights Reserved.

#include "WeaponEquipAbility.h"
#include "AbilityCharacter.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "Core/AbilitySystem/CombatGameplayTags.h"
#include "Features/Weapons/Equipment/WeaponEquipmentComponent.h"
#include "Features/Weapons/WeaponData.h"

UWeaponEquipAbility::UWeaponEquipAbility()
{
	// equip identity: input bindings, Blueprints and AI activate through
	// Ability.Weapon.Equip
	SetAssetTags(FGameplayTagContainer(TAG_Ability_Weapon_Equip));

	// the ability activates from the equip input press event
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = TAG_Event_InputEquipPressed;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

	// equipping needs a weapon on the character; neither firing nor reloading
	// may coexist with a swap
	ActivationRequiredTags.AddTag(TAG_State_Weapon_Equipped);
	ActivationBlockedTags.AddTag(TAG_State_Weapon_Reloading);
}

bool UWeaponEquipAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// nothing equipped: nothing to draw or holster
	return UWeaponEquipmentComponent::GetEquippedWeaponOnActor(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr) != nullptr;
}

void UWeaponEquipAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UWeaponEquipmentComponent* Equipment = UWeaponEquipmentComponent::FindEquipmentOnActor(GetAvatarActorFromActorInfo());
	const UWeaponData* WeaponData = Equipment ? Equipment->GetEquippedWeaponData() : nullptr;

	if (!Equipment || !WeaponData || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// the same ability toggles: drawn weapons holster, holstered ones draw
	bDrawing = !Equipment->IsWeaponDrawn();

	// blocking state for the swap's duration: fire and reload gate on it
	FGameplayTagContainer SwappingTags;
	SwappingTags.AddTag(TAG_State_Weapon_Swapping);
	ApplyStateSlot(TAG_State_Weapon_Swapping, SwappingTags);

	UAnimMontage* Montage = bDrawing ? WeaponData->EquipMontage : WeaponData->UnequipMontage;

	if (Montage)
	{
		MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Montage);
		MontageTask->OnCompleted.AddDynamic(this, &UWeaponEquipAbility::OnMontageFinished);
		MontageTask->OnInterrupted.AddDynamic(this, &UWeaponEquipAbility::OnMontageFinished);
		MontageTask->OnCancelled.AddDynamic(this, &UWeaponEquipAbility::OnMontageFinished);

		// the swap notify is the frame the hand reaches the weapon
		SwapEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TAG_Event_Weapon_Swap);
		SwapEventTask->EventReceived.AddDynamic(this, &UWeaponEquipAbility::OnSwapEvent);

		MontageTask->ReadyForActivation();
		SwapEventTask->ReadyForActivation();
	}
	else
	{
		// montageless setups swap instantly (a first bring-up without any
		// equip animation: the weapon appears in hand and the layer links)
		if (bDrawing)
		{
			Equipment->DrawWeapon();
		}
		else
		{
			Equipment->HolsterWeapon();
		}

		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UWeaponEquipAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// clean the blocking state whether the swap completed or was interrupted
	ClearStateSlot(TAG_State_Weapon_Swapping);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UWeaponEquipAbility::OnSwapEvent(FGameplayEventData EventData)
{
	// the hand reached the weapon: the equipment component does the attach,
	// layer link and state tag swap. It runs on the server (it owns all
	// weapon state); the client's cosmetic OnReps follow the replication
	UWeaponEquipmentComponent* Equipment = UWeaponEquipmentComponent::FindEquipmentOnActor(GetAvatarActorFromActorInfo());
	if (!Equipment)
	{
		return;
	}

	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (Avatar && Avatar->HasAuthority())
	{
		if (bDrawing)
		{
			Equipment->DrawWeapon();
		}
		else
		{
			Equipment->HolsterWeapon();
		}
	}
}

void UWeaponEquipAbility::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
