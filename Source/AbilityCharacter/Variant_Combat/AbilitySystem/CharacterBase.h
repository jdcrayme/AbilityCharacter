// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CombatAttacker.h"
#include "CombatDamageable.h"
#include "Animation/AnimMontage.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Engine/TimerHandle.h"
#include "AbilitySystem/CharacterAttributeSet.h"
#include "CharacterBase.generated.h"

USTRUCT(BlueprintType)
struct FInputActionAbilityMapping
{
	GENERATED_BODY()
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UInputAction* InputAction = nullptr;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag AbilityTag;
};

// Declare delegate with one int32 parameter
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAbilitiesChanged);

/**
 *  An AI-controlled character with combat capabilities.
 *  Its bundled AI Controller runs logic through StateTree
 */
UCLASS(abstract)
class ACharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	ACharacterBase();

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	EGameplayEffectReplicationMode AbilitySystemReplicationMode = EGameplayEffectReplicationMode::Mixed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	class UCharacterAttributeSet* AttributeSet;

	UPROPERTY(EditAnywhere, Category = "Input")
	TArray<FInputActionAbilityMapping> AbilityInputMappings;
 
	/** The handler function that will receive the payload */
	void TryActivateAbilityByTag(FGameplayTag AbilityTag);
	
	// Event dispatcher exposed to Blueprints and C++
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAbilitiesChanged OnAbilitiesChanged;
	
protected:
	
	/** Handles input bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void GrantAbility(TSubclassOf<UGameplayAbility> AbilityClass, int32 AbilityLevel);

	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void RemoveAbility(const FGameplayAbilitySpecHandle& Handle);

	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;

	virtual void OnRep_PlayerState() override;

};
