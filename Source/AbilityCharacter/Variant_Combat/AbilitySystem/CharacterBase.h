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
#include "NativeGameplayTags.h"
#include "CharacterBase.generated.h"


UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_DataDamage);

class UInputAction;

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

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Ability System")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
	EGameplayEffectReplicationMode AbilitySystemReplicationMode = EGameplayEffectReplicationMode::Mixed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System")
	class UCharacterAttributeSet* AttributeSet;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability System|Attributes")
	TSubclassOf<UGameplayEffect> DefaultAttributeEffect;
	
	UPROPERTY(EditAnywhere, Category = "Ability System|Input")
	TArray<FInputActionAbilityMapping> AbilityInputMappings;
 	
	// Event dispatcher exposed to Blueprints and C++
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAbilitiesChanged OnAbilitiesChanged;
	
	UPROPERTY(EditDefaultsOnly, Category="GAS")
	TSubclassOf<class UGameplayEffect> DamageGEClass;
protected:
	
	void InitializeAttributes();
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void BeginPlay() override;
	
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	virtual void PossessedBy(AController* NewController) override;

	virtual void OnRep_PlayerState() override;
	
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void TryActivateAbilityByTag(FGameplayTag AbilityTag);
	
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void GrantAbility(TSubclassOf<UGameplayAbility> AbilityClass, int32 AbilityLevel);

	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void RemoveAbility(const FGameplayAbilitySpecHandle& Handle);

	UFUNCTION(BlueprintGetter, Category = "Ability")
	float GetHealth() const {return AttributeSet->GetHealth();}

};
