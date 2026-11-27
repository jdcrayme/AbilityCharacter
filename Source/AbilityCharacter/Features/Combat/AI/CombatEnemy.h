// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Core/Interfaces/CombatDamageable.h"
#include "Core/Characters/CharacterBase.h"
#include "Engine/TimerHandle.h"
#include "CombatEnemy.generated.h"

/** Completed attack animation delegate for StateTree */
DECLARE_DELEGATE(FOnEnemyAttackCompleted);

/** Landed delegate for StateTree */
DECLARE_DELEGATE(FOnEnemyLanded);

/** Enemy died delegate */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyDied);

/**
 *  An AI-controlled character with combat capabilities.
 *  Its bundled AI Controller runs logic through StateTree, and all attacks are
 *  GameplayAbilities activated by the StateTree tasks through ability tags.
 */
UCLASS(abstract)
class ABILITYCHARACTER_API ACombatEnemy : public ACharacterBase
{
	GENERATED_BODY()

public:

	/** Constructor */
	ACombatEnemy();

protected:

	/** Time to wait before removing this character from the level after it dies */
	UPROPERTY(EditAnywhere, Category="Death")
	float DeathRemovalTime = 5.0f;

	/** Enemy death timer */
	FTimerHandle DeathTimer;

	/** Last recorded location we're being attacked from */
	FVector LastDangerLocation = FVector::ZeroVector;

	/** Last recorded game time we were attacked */
	float LastDangerTime = -1000.0f;

	/** True while an attack ability is running */
	bool bIsAttacking = false;

public:
	/** Attack completed internal delegate to notify StateTree tasks */
	FOnEnemyAttackCompleted OnAttackCompleted;

	/** Landed internal delegate to notify StateTree tasks. We use this instead of the built-in Landed delegate so we can bind to a Lambda in StateTree tasks */
	FOnEnemyLanded OnEnemyLanded;

	/** Enemy died delegate. Allows external subscribers to respond to enemy death */
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnEnemyDied OnEnemyDied;

public:

	/** Performs an AI-initiated combo attack by activating the combo attack ability. Number of hits will be decided by the ability */
	void DoAIComboAttack();

	/** Performs an AI-initiated charged attack by activating the charged attack ability. Charge time will be decided by the ability */
	void DoAIChargedAttack();

	/** Called when an attack ability finishes. Resets the attacking state and notifies the StateTree */
	UFUNCTION()
	void OnAttackAbilityFinished();

	/** Returns the last recorded location we were attacked from */
	const FVector& GetLastDangerLocation() const;

	/** Returns the last game time we were attacked */
	float GetLastDangerTime() const;

public:

	// ~begin ICombatDamageable interface

	/** Allows the enemy to react to incoming attacks */
	virtual void NotifyDanger(const FVector& DangerLocation, AActor* DangerSource) override;

	/** Handles death events */
	virtual void HandleDeath() override;

	// ~end ICombatDamageable interface

protected:

	/** Removes this character from the level after it dies */
	void RemoveFromLevel();

	/** Activates the attack ability matching the tag and tracks the attacking state; returns true while the attack is running */
	bool TryAIAttack(const FGameplayTag& AbilityTag);

public:

	/** Overrides landing to reset damage ragdoll physics and notify the StateTree */
	virtual void Landed(const FHitResult& Hit) override;

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** EndPlay cleanup */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
};
