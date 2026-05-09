// Copyright Epic Games, Inc. All Rights Reserved.


#include "CharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CombatAIController.h"
#include "Components/WidgetComponent.h"
#include "Engine/DamageEvents.h"
#include "CombatLifeBar.h"
#include "EnhancedInputComponent.h"
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_DataDamage, "Data.Damage");

ACharacterBase::ACharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(AbilitySystemReplicationMode);
	
	AttributeSet = CreateDefaultSubobject<class UCharacterAttributeSet>(TEXT("AttributeSet"));
}

void ACharacterBase::InitializeAttributes()
{
	if (AbilitySystemComponent && DefaultAttributeEffect)
	{
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);
 
		// Level 1 applies base values from the GE
		FGameplayEffectSpecHandle NewHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributeEffect, 1.f, EffectContext);
		if (NewHandle.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*NewHandle.Data.Get());
		}
	}
}

void ACharacterBase::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
		
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Gameplay Ability Bindings
		for (const FInputActionAbilityMapping& Mapping : AbilityInputMappings)
		{
			if (Mapping.InputAction && Mapping.AbilityTag.IsValid())
			{
				// We bind the action and pass the AbilityTag as a payload to the function
				EnhancedInputComponent->BindAction(
					Mapping.InputAction, 
					ETriggerEvent::Started, 
					this, 
					&ThisClass::TryActivateAbilityByTag, 
					Mapping.AbilityTag // This is the payload
				);
			}
		}
	}
}
	
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	InitializeAttributes();
}

float ACharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (AbilitySystemComponent && DamageGEClass && DamageAmount > 0.f)
	{
		// "Damage" tag used for SetByCaller, must match what you reference in your GameplayEffect as the SetByCaller key
		//static const FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
 
		// Create EffectSpec at level 1
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);
 
		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DamageGEClass, 1.f, EffectContext);
		if (SpecHandle.IsValid())
		{
			// Pass the actual damage via SetByCaller so it's not hard-coded in the GE
			SpecHandle.Data->SetSetByCallerMagnitude(TAG_DataDamage, -DamageAmount);
 
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
		}
		return DamageAmount;
	}
 
	// Fallback to default if no GAS or GE class set
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void ACharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (AbilitySystemComponent)
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

void ACharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	if (AbilitySystemComponent)
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

void ACharacterBase::TryActivateAbilityByTag(FGameplayTag AbilityTag)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag));
}

UAbilitySystemComponent* ACharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ACharacterBase::GrantAbility(TSubclassOf<UGameplayAbility> AbilityClass, int32 AbilityLevel)
{
	if (AbilitySystemComponent && HasAuthority() && AbilityClass)
	{
		FGameplayAbilitySpec AbilitySpec(AbilityClass, AbilityLevel, INDEX_NONE, this);
		FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->GiveAbility(AbilitySpec);

		OnAbilitiesChanged.Broadcast();
		
	}
}

void ACharacterBase::RemoveAbility(const FGameplayAbilitySpecHandle& Handle)
{
	if (AbilitySystemComponent && HasAuthority())
	{
		AbilitySystemComponent->ClearAbility(Handle);		
		OnAbilitiesChanged.Broadcast();
	}
}