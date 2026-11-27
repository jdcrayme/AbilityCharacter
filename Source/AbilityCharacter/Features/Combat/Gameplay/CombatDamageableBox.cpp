// Copyright Epic Games, Inc. All Rights Reserved.


#include "CombatDamageableBox.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Core/AbilitySystem/CombatGameplayTags.h"
#include "Core/AbilitySystem/CombatEffectContext.h"
#include "UObject/ConstructorHelpers.h"

ACombatDamageableBox::ACombatDamageableBox()
{
	PrimaryActorTick.bCanEverTick = false;

	// create the mesh
	RootComponent = Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	// set the collision properties
	Mesh->SetCollisionProfileName(FName("BlockAllDynamic"));

	// enable physics
	Mesh->SetSimulatePhysics(true);

	// disable navigation relevance so boxes don't affect NavMesh generation
	Mesh->bNavigationRelevant = false;

	// create the ability system component so abilities can hit the box with standard damage effects
	AbilitySystemComponent = CreateDefaultSubobject<UCombatAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UCharacterAttributeSet>(TEXT("AttributeSet"));
	AbilitySystemComponent->AddAttributeSetSubobject(AttributeSet);

	// effect used to convert non-GAS damage sources into attribute changes
	static ConstructorHelpers::FClassFinder<UGameplayEffect> DamageEffectFinder(TEXT("/Game/Variant_Combat/Effect/GE_DamageHealth"));
	if (DamageEffectFinder.Succeeded())
	{
		DamageEffectClass = DamageEffectFinder.Class;
	}
}

void ACombatDamageableBox::BeginPlay()
{
	Super::BeginPlay();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	// start with the configured HP
	AbilitySystemComponent->SetNumericAttributeBase(UCharacterAttributeSet::GetHealthAttribute(), CurrentHP);

	// watch for the box running out of health
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCharacterAttributeSet::GetHealthAttribute())
		.AddUObject(this, &ACombatDamageableBox::OnHealthChanged);
}

void ACombatDamageableBox::OnHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	// are we dead?
	if (ChangeData.NewValue <= 0.0f)
	{
		HandleDeath();
	}
}

void ACombatDamageableBox::HandleGameplayCue(UObject* Self, FGameplayTag GameplayCueTag, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters)
{
	if (GameplayCueTag.MatchesTagExact(TAG_GameplayCue_Damage) && EventType == EGameplayCueEvent::Executed)
	{
		// react to the damage cue: apply the impulse and let Blueprint play effects at the impact
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
			Mesh->AddImpulseAtLocation(KnockbackImpulse * Mesh->GetMass(), ImpactLocation);
		}

		// call the BP handler to play effects, etc.
		OnBoxDamaged(ImpactLocation, KnockbackImpulse);
	}
}

void ACombatDamageableBox::RemoveFromLevel()
{
	// destroy this actor
	Destroy();
}

void ACombatDamageableBox::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the death timer
	GetWorld()->GetTimerManager().ClearTimer(DeathTimer);
}

void ACombatDamageableBox::ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse)
{
	if (!AbilitySystemComponent || Damage <= 0.0f || !DamageEffectClass) return;

	// convert the non-GAS damage source into a standard damage effect
	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(DamageCauser);

	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DamageEffectClass, 1.f, EffectContext);
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(TAG_DataDamage, -Damage);
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
	}

	// apply a physics impulse to the box, ignoring its mass
	Mesh->AddImpulseAtLocation(DamageImpulse * Mesh->GetMass(), DamageLocation);

	// call the BP handler to play effects, etc.
	OnBoxDamaged(DamageLocation, DamageImpulse);
}

void ACombatDamageableBox::HandleDeath()
{
	// change the collision object type to Visibility so we ignore most interactions but still retain physics collisions
	Mesh->SetCollisionObjectType(ECC_Visibility);

	// call the BP handler to play effects, etc.
	OnBoxDestroyed();

	// set up the death cleanup timer
	GetWorld()->GetTimerManager().SetTimer(DeathTimer, this, &ACombatDamageableBox::RemoveFromLevel, DeathDelayTime);
}

void ACombatDamageableBox::ApplyHealing(float Healing, AActor* Healer)
{
	// stub
}

void ACombatDamageableBox::NotifyDanger(const FVector& DangerLocation, AActor* DangerSource)
{
	// stub
}
