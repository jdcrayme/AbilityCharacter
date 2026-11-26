// Copyright Epic Games, Inc. All Rights Reserved.


#include "CharacterBase.h"
#include "AbilitySystemBlueprintLibrary.h"
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
#include "CombatEffectContext.h"
#include "AbilitySystem/UseAbility.h"

ACharacterBase::ACharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UCombatAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(AbilitySystemReplicationMode);

	// partial ragdoll reactions keep the pelvis upright
	PelvisBoneName = TEXT("pelvis");

	// the passive use ability is granted to every combat character; it only shows
	// UI for locally controlled players
	DefaultUseAbility = UUseAbility::StaticClass();
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

void ACharacterBase::GrantDefaultAbilities()
{
	if (!HasAuthority()) return;

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities)
	{
		if (AbilityClass)
		{
			GrantAbility(AbilityClass, 1);
		}
	}

	// the passive use ability is granted separately so Blueprint subclasses that
	// override the DefaultAbilities array still get interactions
	if (DefaultUseAbility)
	{
		GrantAbility(DefaultUseAbility, 1);
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
	GrantDefaultAbilities();

	if (AbilitySystemComponent)
	{
		// the attribute set owns death: react to the State.Dead tag it applies
		AbilitySystemComponent->RegisterGameplayTagEvent(Tag_State_Dead, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &ACharacterBase::OnDeadTagChanged);
	}
}

float ACharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float retval = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (AbilitySystemComponent && DamageGEClass && DamageAmount > 0.f)
	{
		// Convert the engine damage event into a GameplayEffect. Attack abilities
		// apply their damage effects directly; this path covers non-GAS sources.
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(DamageCauser);

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DamageGEClass, 1.f, EffectContext);
		if (SpecHandle.IsValid())
		{
			// Pass the actual damage via SetByCaller so it's not hard-coded in the GE
			SpecHandle.Data->SetSetByCallerMagnitude(TAG_DataDamage, -DamageAmount);

			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);

			// run the damage cue so the character reacts (no knockback on this path)
			FGameplayCueParameters CueParams;
			CueParams.EffectContext = EffectContext;
			CueParams.RawMagnitude = DamageAmount;
			CueParams.Instigator = DamageCauser;
			CueParams.Location = GetActorLocation();
			AbilitySystemComponent->ExecuteGameplayCue(TAG_GameplayCue_Damage, CueParams);
		}
		return retval;
	}

	// Fallback to default if no GAS or GE class set
	return retval;
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

void ACharacterBase::HandleGameplayCue(UObject* Self, FGameplayTag GameplayCueTag, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters)
{
	if (GameplayCueTag.MatchesTagExact(TAG_GameplayCue_Damage) && EventType == EGameplayCueEvent::Executed)
	{
		HandleDamageCue(Parameters);
	}
}

void ACharacterBase::HandleDamageCue(const FGameplayCueParameters& CueParameters)
{
	// dead characters are already ragdolling, don't fight the physics
	if (AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(Tag_State_Dead))
	{
		return;
	}

	const FCombatGameplayEffectContext* CombatContext = CombatEffectContext::Extract(CueParameters.EffectContext);

	FVector ImpactLocation = CueParameters.Location;
	FVector ImpactNormal = CueParameters.Normal;
	if (CombatContext && CombatContext->GetHitResult())
	{
		ImpactLocation = CombatContext->GetHitResult()->ImpactPoint;
		ImpactNormal = CombatContext->GetHitResult()->ImpactNormal;
	}

	// apply the knockback impulse the attacking ability placed in the effect context
	if (CombatContext && !CombatContext->GetKnockbackImpulse().IsZero())
	{
		GetCharacterMovement()->AddImpulse(CombatContext->GetKnockbackImpulse(), true);

		if (GetMesh()->IsSimulatingPhysics())
		{
			GetMesh()->AddImpulseAtLocation(CombatContext->GetKnockbackImpulse() * GetMesh()->GetMass(), ImpactLocation);
		}
	}

	// interrupt any attack montage we were playing
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(0.1f);
	}

	// enable partial ragdoll physics, but keep the pelvis vertical
	GetMesh()->SetPhysicsBlendWeight(0.5f);
	GetMesh()->SetBodySimulatePhysics(PelvisBoneName, false);

	// Blueprint feedback (VFX, camera shake)
	ReceivedDamage(CueParameters.RawMagnitude, ImpactLocation, (-ImpactNormal).GetSafeNormal());
}

void ACharacterBase::OnDeadTagChanged(FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0)
	{
		HandleDeath();
	}
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

void ACharacterBase::SendAbilityInputEvent(FGameplayTag EventTag)
{
	if (!EventTag.IsValid()) return;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventTag, FGameplayEventData());
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

void ACharacterBase::ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse)
{
	// convert the non-GAS damage source into the standard damage pipeline
	FDamageEvent DamageEvent;
	const float ActualDamage = TakeDamage(Damage, DamageEvent, nullptr, DamageCauser);

	// only process knockback if we received nonzero damage
	if (ActualDamage > 0.0f)
	{
		GetCharacterMovement()->AddImpulse(DamageImpulse, true);

		if (GetMesh()->IsSimulatingPhysics())
		{
			GetMesh()->AddImpulseAtLocation(DamageImpulse * GetMesh()->GetMass(), DamageLocation);
		}
	}
}

void ACharacterBase::HandleDeath()
{
	// base has no death presentation; character subclasses handle it
}

void ACharacterBase::ApplyHealing(float Healing, AActor* Healer)
{
	// stub
}

void ACharacterBase::NotifyDanger(const FVector& DangerLocation, AActor* DangerSource)
{
	// stub, AI characters record danger for their StateTree
}

void ACharacterBase::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (AbilitySystemComponent && !AbilitySystemComponent->HasMatchingGameplayTag(Tag_State_Dead))
	{
		// disable ragdoll physics
		GetMesh()->SetPhysicsBlendWeight(0.0f);
	}
}
