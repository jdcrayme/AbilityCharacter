// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Features/Weapons/AbilitySystem/WeaponAbilityBase.h"
#include "WeaponEquipAbility.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
struct FGameplayEventData;

/**
 *  Weapon equip ability: draws a holstered weapon and holsters a drawn one
 *  (the same ability toggles both directions, so input, Blueprints and AI
 *  only ever need Ability.Weapon.Equip).
 *
 *  Flow:
 *  1. Activates from the equip input event (or by the Ability.Weapon.Equip tag).
 *  2. Plays the weapon's equip (or unequip) montage and applies the
 *     State.Weapon.Swapping tags, which block fire and reload mid-swap.
 *  3. The montage's AnimNotify_SendGameplayEvent (Event.Weapon.Swap) is the
 *     frame the hand reaches the weapon: the equipment component then
 *     attaches it to the hand (or back) socket, links (or unlinks) the
 *     weapon's overlay anim layer and swaps the State.Weapon slot tags.
 *  4. The montage ends and the ability cleans its tags up.
 *
 *  The component does the mechanical work at the swap frame; the ability
 *  owns timing, gating and animation, exactly the split the stance system
 *  uses. Montageless weapons (or a first test setup) draw instantly.
 */
UCLASS()
class ABILITYCHARACTER_API UWeaponEquipAbility : public UWeaponAbilityBase
{
	GENERATED_BODY()

public:

	UWeaponEquipAbility();

protected:

	/** Requires an equipped weapon (something to draw or holster) */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/** The swap notify: the hand reaches the weapon, the equipment swaps it */
	UFUNCTION()
	virtual void OnSwapEvent(FGameplayEventData EventData);

	/** Montage finished, interrupted or cancelled */
	UFUNCTION()
	virtual void OnMontageFinished();

protected:

	/** Task playing the equip or unequip montage */
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	/** Task listening for the swap event from the montage notifies */
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> SwapEventTask;

	/** Whether this activation draws (true) or holsters (false) */
	bool bDrawing = true;
};
