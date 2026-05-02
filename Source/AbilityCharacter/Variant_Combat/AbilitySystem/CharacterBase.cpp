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

ACharacterBase::ACharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(AbilitySystemReplicationMode);
	AttributeSet = CreateDefaultSubobject<class UCharacterAttributeSet>(TEXT("AttributeSet"));
}

void ACharacterBase::TryActivateAbilityByTag(FGameplayTag AbilityTag)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag));
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

void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
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
