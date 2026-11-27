// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/Interfaces/CombatDamageable.h"
#include "GameplayCueInterface.h"
#include "Core/AbilitySystem/CombatAbilitySystemComponent.h"
#include "Core/AbilitySystem/CharacterAttributeSet.h"
#include "CombatDummy.generated.h"

class UStaticMeshComponent;
class UPhysicsConstraintComponent;

/**
 *  A simple invincible combat training dummy.
 *  Carries an ability system component so attack abilities can hit it with
 *  standard damage effects; its knockback reaction runs through the
 *  GameplayCue.Damage cue.
 */
UCLASS(abstract)
class ABILITYCHARACTER_API ACombatDummy : public AActor, public ICombatDamageable, public IGameplayCueInterface
{
	GENERATED_BODY()

	/** Root component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;

	/** Static base plate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* BasePlate;

	/** Physics enabled dummy mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Dummy;

	/** Physics constraint holding the dummy and base plate together */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UPhysicsConstraintComponent* PhysicsConstraint;

	/** Ability system component receiving damage effects */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCombatAbilitySystemComponent* AbilitySystemComponent;

	/** Attribute set backing the dummy's (effectively infinite) health */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCharacterAttributeSet* AttributeSet;

public:

	/** Constructor */
	ACombatDummy();

	// ~Begin CombatDamageable interface

		/** Handles damage from non-GAS sources */
	virtual void ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse) override;

	/** Handles death events */
	virtual void HandleDeath() override;

	/** Handles healing events */
	virtual void ApplyHealing(float Healing, AActor* Healer) override;

	/** Allows reaction to incoming attacks */
	virtual void NotifyDanger(const FVector& DangerLocation, AActor* DangerSource) override;

	// ~End CombatDamageable interface

	// ~Begin IGameplayCueInterface

	virtual void HandleGameplayCue(UObject* Self, FGameplayTag GameplayCueTag, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters) override;

	// ~End IGameplayCueInterface

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Blueprint handle to apply damage effects */
	UFUNCTION(BlueprintImplementableEvent, Category="Combat", meta = (DisplayName = "On Dummy Damaged"))
	void BP_OnDummyDamaged(const FVector& Location, const FVector& Direction);
};
