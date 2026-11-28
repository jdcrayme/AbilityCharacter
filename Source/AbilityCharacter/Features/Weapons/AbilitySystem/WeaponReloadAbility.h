// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Features/Weapons/AbilitySystem/WeaponAbilityBase.h"
#include "WeaponReloadAbility.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
struct FGameplayEventData;

/**
 *  Weapon reload ability. The magazine is the weapon's operational state and
 *  the reserve is the character's inventory, so a reload is a transfer
 *  between the two pools, and the montage decides when it happens:
 *
 *  1. Activates from the reload input event (or by the Ability.Weapon.Reload tag).
 *  2. Plays the weapon's reload montage and applies State.Weapon.Reloading,
 *     which blocks fire and equip while it runs.
 *  3. The reload montage's AnimNotify_SendGameplayEvent (Event.Weapon.MagIn)
 *     is the frame the magazine seats: the server moves min(magazine room,
 *     reserve) rounds out of the inventory's reserve entry and into the
 *  4. The montage ends (completes or is interrupted - damage interrupts
 *     montages, which cancels the reload exactly like it cancels attacks)
 *     and the ability cleans its tags up.
 *
 *  Because the transfer happens at the MagIn notify rather than at montage
 *  end, an interrupted reload keeps whatever rounds already seated - partial
 *  reloads fall out of the data flow for free.
 */
UCLASS()
class ABILITYCHARACTER_API UWeaponReloadAbility : public UWeaponAbilityBase
{
	GENERATED_BODY()

public:

	UWeaponReloadAbility();

protected:

	/** Requires a drawn weapon with room in the magazine and rounds in the reserve */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/** The magazine-in notify: the server moves rounds from the reserve into the magazine */
	UFUNCTION()
	virtual void OnMagInEvent(FGameplayEventData EventData);

	/** Montage finished, interrupted or cancelled */
	UFUNCTION()
	virtual void OnMontageFinished();

	/** Runs the reserve -> magazine transfer; server instance only */
	void TransferAmmo();

protected:

	/** Task playing the reload montage */
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	/** Task listening for the magazine-in event from the montage notifies */
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> MagInTask;
};
