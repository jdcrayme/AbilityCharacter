// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/Interfaces/CombatDamageable.h"
#include "GameplayCueInterface.h"
#include "Core/AbilitySystem/CombatAbilitySystemComponent.h"
#include "Core/AbilitySystem/CharacterAttributeSet.h"
#include "CombatDamageableBox.generated.h"

class UStaticMeshComponent;
class UGameplayEffect;

/**
 *  A simple physics box that takes damage through the ability system.
 *  Attack abilities apply standard damage effects to its attribute set; death
 *  is detected by watching the Health attribute.
 */
UCLASS(abstract)
class ABILITYCHARACTER_API ACombatDamageableBox : public AActor, public ICombatDamageable, public IGameplayCueInterface
{
	GENERATED_BODY()

	/** Damageable box mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Mesh;

	/** Ability system component receiving damage effects */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCombatAbilitySystemComponent* AbilitySystemComponent;

	/** Attribute set backing the box's health */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCharacterAttributeSet* AttributeSet;

public:

	/** Constructor */
	ACombatDamageableBox();

protected:

	/** Amount of HP this box starts with. Applied to the Health attribute on BeginPlay */
	UPROPERTY(EditAnywhere, Category="Damage")
	float CurrentHP = 3.0f;

	/** Gameplay effect used to convert non-GAS damage sources into attribute changes */
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/** Time to wait before we remove this box from the level. */
	UPROPERTY(EditAnywhere, Category="Damage", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float DeathDelayTime = 6.0f;

	/** Timer to defer destruction of this box after its HP are depleted */
	FTimerHandle DeathTimer;

	/** Blueprint damage handler for effect playback */
	UFUNCTION(BlueprintImplementableEvent, Category="Damage")
	void OnBoxDamaged(const FVector& DamageLocation, const FVector& DamageImpulse);

	/** Blueprint destruction handler for effect playback */
	UFUNCTION(BlueprintImplementableEvent, Category="Damage")
	void OnBoxDestroyed();

	/** Death detection: reacts to the Health attribute changing */
	void OnHealthChanged(const FOnAttributeChangeData& ChangeData);

	/** Timer callback to remove the box from the level after it dies */
	void RemoveFromLevel();

public:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** EndPlay cleanup */
	void EndPlay(EEndPlayReason::Type EndPlayReason) override;

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

	/** Reacts to the GameplayCue.Damage cue: applies the knockback impulse and notifies Blueprint */
	virtual void HandleGameplayCue(UObject* Self, FGameplayTag GameplayCueTag, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters) override;

	// ~End IGameplayCueInterface
};
