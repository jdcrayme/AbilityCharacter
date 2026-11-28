// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Features/Combat/AbilitySystem/CombatAbilityBase.h"
#include "WeaponFireAbility.generated.h"

/**
 *  Weapon-driven fire ability: the same montage -> trace notify -> effect ->
 *  cue chain UCombatAbilityBase runs for attacks, with every number (trace
 *  distance, damage, knockback, fire interval, muzzle socket, damage effect)
 *  coming from the equipped weapon's UWeaponData at runtime instead of from
 *  ability defaults. One instance of this class serves every hitscan weapon
 *  ever authored: a pistol and a crossbow are two data assets.
 *
 *  Flow per shot:
 *  1. Activates from the fire input event (or by the Ability.Weapon.Fire tag).
 *  2. Commits (ammo is the cost; optional stamina; cooldown = Data.Cooldown
 *     SetByCaller fed with the weapon's fire interval).
 *  3. Plays the weapon's fire montage; the attack trace notify (or montage
 *     end when there is none) is the fire frame.
 *  4. Consumes one round from the weapon's magazine (server).
 *  5. Runs the hitscan from the player's view (or the weapon's muzzle for
 *     AI), applies the weapon's damage effect and executes the damage cue on
 *     the victim, exactly like URangedAbility does.
 *
 *  Gating: requires State.Weapon.Drawn, blocked while reloading or swapping,
 *  and refuses to activate on an empty magazine.
 */
UCLASS()
class ABILITYCHARACTER_API UWeaponFireAbility : public UCombatAbilityBase
{
	GENERATED_BODY()

public:

	UWeaponFireAbility();

protected:

	/** Requires a drawn weapon and a round in the magazine; base checks handle the rest */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	/** Pulls the fire montage from the equipped weapon, then runs the base montage chain */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** Ammo is the fire ability's cost: no stamina drain */
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	/** Cooldown = the weapon's fire interval, fed into the weapon's cooldown effect through the Data.Cooldown SetByCaller */
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	/** The fire frame: run the weapon's hitscan once */
	virtual void OnAttackTraceEvent(FGameplayEventData EventData) override;

	/** Montages without a trace notify still fire once, at the end */
	virtual void OnMontageFinished() override;

	/** Hitscan from the view or the weapon's muzzle with the weapon data's parameters */
	void PerformWeaponTrace();

	/** Applies the weapon data's damage effect and the damage cue to a hit target */
	void ApplyWeaponDamage(const FHitResult& HitResult, const struct FWeaponFiringData& Firing);

	/** Blueprint feedback for the fire frame: muzzle flash, sounds, camera shake. Runs wherever the fire frame lands */
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon|Fire")
	void OnWeaponFired(const FTransform& MuzzleTransform);

	/** Blueprint feedback for the empty-magazine click (server instance) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon|Fire")
	void OnMagEmpty();

	/** Whether the fire frame already fired this activation */
	bool bHasFired = false;
};
