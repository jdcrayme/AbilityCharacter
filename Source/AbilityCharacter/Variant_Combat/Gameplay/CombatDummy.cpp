// Copyright Epic Games, Inc. All Rights Reserved.


#include "CombatDummy.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "CombatEffectContext.h"
#include "CombatGameplayTags.h"
#include "Engine/DamageEvents.h"

ACombatDummy::ACombatDummy()
{
 	PrimaryActorTick.bCanEverTick = true;

	// create the root
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// create the base plate
	BasePlate = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base Plate"));
	BasePlate->SetupAttachment(RootComponent);

	// create the dummy
	Dummy = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Dummy"));
	Dummy->SetupAttachment(RootComponent);

	Dummy->SetSimulatePhysics(true);

	// create the physics constraint
	PhysicsConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("Physics Constraint"));
	PhysicsConstraint->SetupAttachment(RootComponent);

	PhysicsConstraint->SetConstrainedComponents(BasePlate, NAME_None, Dummy, NAME_None);

	// create the ability system component so abilities can hit the dummy with standard damage effects
	AbilitySystemComponent = CreateDefaultSubobject<UCombatAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UCharacterAttributeSet>(TEXT("AttributeSet"));
	AbilitySystemComponent->AddAttributeSetSubobject(AttributeSet);
}

void ACombatDummy::BeginPlay()
{
	Super::BeginPlay();

	// the dummy is invincible: give it effectively infinite health (HealthMax is the clamp)
	AbilitySystemComponent->SetNumericAttributeBase(UCharacterAttributeSet::GetHealthMaxAttribute(), TNumericLimits<float>::Max());
	AbilitySystemComponent->SetNumericAttributeBase(UCharacterAttributeSet::GetHealthAttribute(), TNumericLimits<float>::Max());
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

void ACombatDummy::ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse)
{
	// apply impulse to the dummy
	Dummy->AddImpulseAtLocation(DamageImpulse, DamageLocation);

	// call the BP handler
	BP_OnDummyDamaged(DamageLocation, DamageImpulse.GetSafeNormal());
}

void ACombatDummy::HandleDeath()
{
	// unused
}

void ACombatDummy::ApplyHealing(float Healing, AActor* Healer)
{
	// unused
}

void ACombatDummy::NotifyDanger(const FVector& DangerLocation, AActor* DangerSource)
{
	// unused
}

void ACombatDummy::HandleGameplayCue(UObject* Self, FGameplayTag GameplayCueTag, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters)
{
	if (GameplayCueTag.MatchesTagExact(TAG_GameplayCue_Damage) && EventType == EGameplayCueEvent::Executed)
	{
		// react to the damage cue: knock the dummy away from the impact
		FVector ImpactLocation = Parameters.Location;
		FVector KnockbackImpulse = FVector::ZeroVector;

		if (const FCombatGameplayEffectContext* CombatContext = CombatEffectContext::Extract(Parameters.EffectContext))
		{
			KnockbackImpulse = CombatContext->GetKnockbackImpulse();
			if (CombatContext->GetHitResult())
			{
				ImpactLocation = CombatContext->GetHitResult()->ImpactPoint;
			}
		}

		if (!KnockbackImpulse.IsZero())
		{
			Dummy->AddImpulseAtLocation(KnockbackImpulse * Dummy->GetMass(), ImpactLocation);
		}

		BP_OnDummyDamaged(ImpactLocation, (-Parameters.Normal).GetSafeNormal());
	}
}
