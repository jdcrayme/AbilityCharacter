// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Features/Weapons/AbilitySystem/WeaponAbilityBase.h"
#include "WeaponAimAbility.generated.h"

class UAbilityTask_WaitGameplayEvent;
class UGameplayEffect;
struct FGameplayEventData;

/**
 *  Weapon aim ability: a persistent hold-style ability in the stance
 *  machinery's image. The aim input press activates it, the release event
 *  ends it, and while it runs it owns the replicated State.Aiming tag that
 *  animation blueprints (aim poses and aim offsets), UI (crosshair) and
 *  other abilities (fire spread tightening) key off. Because the state is a
 *  tag and not a camera hack, an AI rifleman activates the same ability and
 *  gets the same overlay for free.
 *
 *  Optional movement effect: the ability applies AimMovementEffect (a walk
 *  speed modifier GE, for instance) while aiming and removes it when aiming
 *  ends. Ends on the aim input release, when the weapon is unequipped or
 *  when the weapon is holstered.
 */
UCLASS()
class ABILITYCHARACTER_API UWeaponAimAbility : public UWeaponAbilityBase
{
	GENERATED_BODY()

public:

	UWeaponAimAbility();

protected:

	/** Requires a drawn weapon */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/** The aim input release ends the aim */
	UFUNCTION()
	virtual void OnAimReleasedEvent(FGameplayEventData EventData);

	/** Ends the aim when the weapon goes away */
	void OnEquippedTagChanged(FGameplayTag Tag, int32 NewCount);

protected:

	/** Optional movement effect applied while aiming (a walk speed modifier, for instance) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Aim")
	TSubclassOf<UGameplayEffect> AimMovementEffect;

	/** Task listening for the aim release event */
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> AimReleaseTask;

	/** Handle of the applied movement effect, removed when aiming ends */
	FActiveGameplayEffectHandle AimEffectHandle;

	/** Registration for the equipped-tag watch that ends the aim on unequip */
	FDelegateHandle EquippedTagEventHandle;
};
