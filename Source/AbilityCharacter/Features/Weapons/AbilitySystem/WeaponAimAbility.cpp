// Copyright Epic Games, Inc. All Rights Reserved.

#include "WeaponAimAbility.h"
#include "AbilityCharacter.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "Core/AbilitySystem/CombatGameplayTags.h"
#include "Features/Weapons/Equipment/WeaponEquipmentComponent.h"

UWeaponAimAbility::UWeaponAimAbility()
{
	// aim identity: input bindings, Blueprints and AI activate through
	// Ability.Weapon.Aim
	SetAssetTags(FGameplayTagContainer(TAG_Ability_Weapon_Aim));

	// the ability activates from the aim input press event
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = TAG_Event_InputAimPressed;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

	// aiming needs a weapon in hand; reloading can't coexist with a steady aim
	ActivationRequiredTags.AddTag(TAG_State_Weapon_Drawn);
	ActivationBlockedTags.AddTag(TAG_State_Weapon_Reloading);

	// a hold-style ability stays active until its release event arrives
	bRetriggerInstancedAbility = false;
}

bool UWeaponAimAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// no equipped weapon: nothing to aim
	return UWeaponEquipmentComponent::GetEquippedWeaponOnActor(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr) != nullptr;
}

void UWeaponAimAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// the aim state goes on through its own slot: the server's instance
	// applies it through the replicated slot (every client sees it), the
	// predicted client instance adds it loose so the pose reacts immediately
	FGameplayTagContainer AimingTags;
	AimingTags.AddTag(TAG_State_Aiming);
	ApplyStateSlot(TAG_State_Aiming, AimingTags);

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (ASC && Avatar && Avatar->HasAuthority() && AimMovementEffect)
	{
		// optional movement modifier while aiming (walk speed, for instance);
		// the server owns movement, so the server instance applies it
		const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(AimMovementEffect, GetAbilityLevel(), ASC->MakeEffectContext());
		if (SpecHandle.IsValid())
		{
			AimEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	// hold-style: the aim ends when the input is released
	AimReleaseTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TAG_Event_InputAimReleased);
	AimReleaseTask->EventReceived.AddDynamic(this, &UWeaponAimAbility::OnAimReleasedEvent);
	AimReleaseTask->ReadyForActivation();

	// and when the weapon goes away (unequipped or holstered through any path)
	if (ASC)
	{
		EquippedTagEventHandle = ASC->RegisterGameplayTagEvent(TAG_State_Weapon_Drawn, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &UWeaponAimAbility::OnEquippedTagChanged);
	}
}

void UWeaponAimAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// drop the aim state tag and the movement effect
	ClearStateSlot(TAG_State_Aiming);

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		if (AimEffectHandle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(AimEffectHandle);
			AimEffectHandle = FActiveGameplayEffectHandle();
		}

		if (EquippedTagEventHandle.IsValid())
		{
			ASC->UnregisterGameplayTagEvent(EquippedTagEventHandle, TAG_State_Weapon_Drawn, EGameplayTagEventType::NewOrRemoved);
			EquippedTagEventHandle.Reset();
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UWeaponAimAbility::OnAimReleasedEvent(FGameplayEventData EventData)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UWeaponAimAbility::OnEquippedTagChanged(FGameplayTag Tag, int32 NewCount)
{
	// weapon holstered or unequipped while aiming: the aim ends with it
	if (NewCount <= 0)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}
