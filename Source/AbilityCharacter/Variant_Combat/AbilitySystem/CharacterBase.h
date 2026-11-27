// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CombatDamageable.h"
#include "CombatInputLabelProvider.h"
#include "GameplayCueInterface.h"
#include "Animation/AnimMontage.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Engine/TimerHandle.h"
#include "AbilitySystem/CharacterAttributeSet.h"
#include "AbilitySystem/CombatAbilitySystemComponent.h"
#include "AbilitySystem/CombatGameplayTags.h"
#include "NativeGameplayTags.h"
#include "CharacterBase.generated.h"

class UInputAction;
class UGameplayAbility;

USTRUCT(BlueprintType)
struct ABILITYCHARACTER_API FInputActionAbilityMapping
{
        GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadOnly)
        TObjectPtr<UInputAction> InputAction = nullptr;

        UPROPERTY(EditAnywhere, BlueprintReadOnly)
        FGameplayTag AbilityTag;
};

/** Generic input action to gameplay event mapping: pressing the input action sends the event tag to the ability system, where any running ability can listen for it (e.g. the passive use ability). Configured per character in Blueprints, so the character class stays free of ability-specific input code. */
USTRUCT(BlueprintType)
struct ABILITYCHARACTER_API FInputActionEventMapping
{
        GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadOnly)
        TObjectPtr<UInputAction> InputAction = nullptr;

        /** Gameplay event tag sent when the input action fires; abilities listen for it with WaitGameplayEvent */
        UPROPERTY(EditAnywhere, BlueprintReadOnly)
        FGameplayTag EventTag;

        /** Key label bound to this event, displayed by prompts and UI (e.g. "E"); keep it in sync with the mapping context. Empty lets the caller fall back to its own default. */
        UPROPERTY(EditAnywhere, BlueprintReadOnly)
        FText KeyLabel;
};

// Event dispatcher exposed to Blueprints and C++
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAbilitiesChanged);

/** Broadcast when an attack ability started by this character finishes. AI StateTree tasks listen for this */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackAbilityEnded);

/**
 *  Base class for every combat character (player and AI).
 *  All attacks run through GameplayAbilities; damage arrives through
 *  GameplayEffects and reactions run through the GameplayCue.Damage cue.
 */
UCLASS(abstract)
class ABILITYCHARACTER_API ACharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatDamageable, public ICombatInputLabelProvider, public IGameplayCueInterface
{
        GENERATED_BODY()

public:

        /** Constructor */
        ACharacterBase();

        UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Ability System")
        UCombatAbilitySystemComponent* AbilitySystemComponent;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
        EGameplayEffectReplicationMode AbilitySystemReplicationMode = EGameplayEffectReplicationMode::Mixed;

        UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability System|Attributes")
        TSubclassOf<UGameplayEffect> DefaultAttributeEffect;

        /** Abilities granted to this character when it initializes */
        UPROPERTY(EditDefaultsOnly, Category = "Ability System|Abilities")
        TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

        UPROPERTY(EditAnywhere, Category = "Ability System|Input")
        TArray<FInputActionAbilityMapping> AbilityInputMappings;

        /** Input actions forwarded to the ability system as generic gameplay events; abilities (e.g. the passive use ability) listen for their event tag */
        UPROPERTY(EditAnywhere, Category = "Ability System|Input")
        TArray<FInputActionEventMapping> EventInputMappings;

        // Event dispatcher exposed to Blueprints and C++
        UPROPERTY(BlueprintAssignable, Category = "Ability System|Events")
        FOnAbilitiesChanged OnAbilitiesChanged;

        /** Broadcast when an attack ability finishes, whether it completed or was interrupted */
        UPROPERTY(BlueprintAssignable, Category = "Ability System|Events")
        FOnAttackAbilityEnded OnAttackAbilityEnded;

        UPROPERTY(EditDefaultsOnly, Category="Ability System")
        TSubclassOf<class UGameplayEffect> DamageGEClass;

        /** Name of the pelvis bone, for damage ragdoll physics */
        UPROPERTY(EditAnywhere, Category="Damage")
        FName PelvisBoneName;

protected:

        void InitializeAttributes();

        /** Grants every ability in DefaultAbilities */
        void GrantDefaultAbilities();

        virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

        virtual void BeginPlay() override;

        virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

        virtual void PossessedBy(AController* NewController) override;

        virtual void OnRep_PlayerState() override;

        /** Reacts to the GameplayCue.Damage cue: knockback, attack interruption, ragdoll blend and Blueprint feedback */
        void HandleDamageCue(const FGameplayCueParameters& CueParameters);

        /** Death is owned by the attribute set through the State.Dead tag */
        void OnDeadTagChanged(FGameplayTag Tag, int32 NewCount);

public:

        virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

        // ~begin IGameplayCueInterface

        virtual void HandleGameplayCue(UObject* Self, FGameplayTag GameplayCueTag, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters) override;

        // ~end IGameplayCueInterface

        UFUNCTION(BlueprintCallable, Category = "Abilities")
        void TryActivateAbilityByTag(FGameplayTag AbilityTag);

        UFUNCTION(BlueprintCallable, Category = "Abilities")
        void GrantAbility(TSubclassOf<UGameplayAbility> AbilityClass, int32 AbilityLevel);

        UFUNCTION(BlueprintCallable, Category = "Abilities")
        void RemoveAbility(const FGameplayAbilitySpecHandle& Handle);

        /** Forwards an input-driven gameplay event to this character's ability system component, activating or informing running abilities */
        UFUNCTION(BlueprintCallable, Category = "Abilities")
        void SendAbilityInputEvent(FGameplayTag EventTag);

        // ~begin CombatInputLabelProvider interface

        /** Returns the key label configured for an event tag in EventInputMappings, or empty text when the tag has no entry or no label set */
        virtual FText GetKeyLabelForEvent_Implementation(FGameplayTag EventTag) override;

        // ~end CombatInputLabelProvider interface

        // ~begin CombatDamageable interface

        /** Handles damage and knockback events coming from non-ability sources */
        virtual void ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse) override;

        /** Handles death events */
        virtual void HandleDeath() override;

        /** Handles healing events */
        virtual void ApplyHealing(float Healing, AActor* Healer) override;

        /** Notifies the actor of impending damage so it can react */
        virtual void NotifyDanger(const FVector& DangerLocation, AActor* DangerSource) override;

        // ~end CombatDamageable interface

        /** Overrides landing to reset damage ragdoll physics */
        virtual void Landed(const FHitResult& Hit) override;

        /** Blueprint handler to play damage dealt effects (called by attack abilities) */
        UFUNCTION(BlueprintImplementableEvent, Category="Combat")
        void DealtDamage(float Damage, const FVector& ImpactPoint);

protected:

        /** Blueprint handler to play damage received effects */
        UFUNCTION(BlueprintImplementableEvent, Category="Combat")
        void ReceivedDamage(float Damage, const FVector& ImpactPoint, const FVector& DamageDirection);
};
