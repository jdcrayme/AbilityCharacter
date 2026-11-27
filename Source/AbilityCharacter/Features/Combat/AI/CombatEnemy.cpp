// Copyright Epic Games, Inc. All Rights Reserved.


#include "CombatEnemy.h"
#include "AbilityCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Features/Combat/AI/CombatAIController.h"
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/AbilitySystem/CombatGameplayTags.h"
#include "Features/Combat/AbilitySystem/AIStanceMonitorAbility.h"
#include "Features/Combat/AbilitySystem/AICombatStanceAbility.h"
#include "Features/Combat/AbilitySystem/NeutralStanceAbility.h"

ACombatEnemy::ACombatEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// tag used by the player's stance monitor to detect nearby combat
	Tags.Add(FName("Enemy"));

	// set the AI Controller class by default
	AIControllerClass = ACombatAIController::StaticClass();

	// use an AI Controller regardless of whether we're placed or spawned
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// ignore the controller's yaw rotation
	bUseControllerRotationYaw = false;

	// set the collision capsule size
	GetCapsuleComponent()->SetCapsuleSize(35.0f, 90.0f);

	// set the character movement properties
	GetCharacterMovement()->bUseControllerDesiredRotation = true;

	// the AI's stance monitor detects the player and the combat stance grants the
	// attack abilities; the StateTree tasks activate them by tag
	DefaultAbilities = {
		UAIStanceMonitorAbility::StaticClass(),
		UAICombatStanceAbility::StaticClass(),
		UNeutralStanceAbility::StaticClass()
	};

	// shared attributes with the player character
	static ConstructorHelpers::FClassFinder<UGameplayEffect> InitialAttributesFinder(TEXT("/Game/Variant_Combat/Effect/GE_InitialCharacter"));
	if (InitialAttributesFinder.Succeeded())
	{
		DefaultAttributeEffect = InitialAttributesFinder.Class;
	}
}

void ACombatEnemy::DoAIComboAttack()
{
	TryAIAttack(TAG_Ability_Attack_Primary);
}

void ACombatEnemy::DoAIChargedAttack()
{
	TryAIAttack(TAG_Ability_Attack_Secondary);
}

bool ACombatEnemy::TryAIAttack(const FGameplayTag& AbilityTag)
{
	// ignore if we're already attacking
	if (bIsAttacking)
	{
		return false;
	}

	// run the attack through the ability system
	const bool bActivated = AbilitySystemComponent && AbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag));
	UE_LOG(LogAbilityCharacter, Verbose, TEXT("[Combat] %s AI attack (%s): TryActivate=%d"), *GetName(), *AbilityTag.ToString(), bActivated);
	bIsAttacking = bActivated;

	if (!bIsAttacking)
	{
		// activation failed (no stamina, stance not active...): complete the task
		// right away so the StateTree doesn't wait on an attack that never starts
		OnAttackCompleted.ExecuteIfBound();
	}

	return bIsAttacking;
}

void ACombatEnemy::OnAttackAbilityFinished()
{
	// reset the attacking flag
	bIsAttacking = false;

	// call the attack completed delegate so the StateTree can continue execution
	OnAttackCompleted.ExecuteIfBound();
}

const FVector& ACombatEnemy::GetLastDangerLocation() const
{
	return LastDangerLocation;
}

float ACombatEnemy::GetLastDangerTime() const
{
	return LastDangerTime;
}

void ACombatEnemy::HandleDeath()
{
	// disable the collision capsule to avoid being hit again while dead
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// disable character movement
	GetCharacterMovement()->DisableMovement();

	// enable full ragdoll physics
	GetMesh()->SetSimulatePhysics(true);

	// call the died delegate to notify any subscribers
	OnEnemyDied.Broadcast();

	// set up the death timer
	GetWorld()->GetTimerManager().SetTimer(DeathTimer, this, &ACombatEnemy::RemoveFromLevel, DeathRemovalTime);
}

void ACombatEnemy::NotifyDanger(const FVector& DangerLocation, AActor* DangerSource)
{
	// ensure we're being attacked by the player
	if (DangerSource && DangerSource->ActorHasTag(FName("Player")))
	{
		// save the danger location and game time
		LastDangerLocation = DangerLocation;
		LastDangerTime = GetWorld()->GetTimeSeconds();
	}
}

void ACombatEnemy::RemoveFromLevel()
{
	// destroy this actor
	Destroy();
}

void ACombatEnemy::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// call the landed Delegate for StateTree
	OnEnemyLanded.ExecuteIfBound();
}

void ACombatEnemy::BeginPlay()
{
	// attributes initialize in BeginPlay, so StateTree sees final values from the start
	Super::BeginPlay();

	// attack abilities broadcast OnAttackAbilityEnded on the character base when they finish
	OnAttackAbilityEnded.AddDynamic(this, &ACombatEnemy::OnAttackAbilityFinished);
}

void ACombatEnemy::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the death timer
	GetWorld()->GetTimerManager().ClearTimer(DeathTimer);
}
