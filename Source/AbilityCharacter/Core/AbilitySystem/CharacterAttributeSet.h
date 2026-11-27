// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "NativeGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "CharacterAttributeSet.generated.h"

/** Tags applied by this attribute set when health or stamina cross their thresholds */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_State_Dead);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_State_Stamina_Tired);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_State_Stamina_Exhausted);

UCLASS()
class ABILITYCHARACTER_API UCharacterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:

	UCharacterAttributeSet();

	//Health
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UCharacterAttributeSet, Health);
	
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UCharacterAttributeSet, Health, OldValue);
	}
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_HealthMax)
	FGameplayAttributeData HealthMax;
	ATTRIBUTE_ACCESSORS_BASIC(UCharacterAttributeSet, HealthMax);

	UFUNCTION()
	void OnRep_HealthMax(const FGameplayAttributeData& OldValue)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UCharacterAttributeSet, HealthMax, OldValue);
	}

	//Stamina
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS_BASIC(UCharacterAttributeSet, Stamina);
	
	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldValue)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UCharacterAttributeSet, Stamina, OldValue);
	}
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_StaminaMax)
	FGameplayAttributeData StaminaMax;
	ATTRIBUTE_ACCESSORS_BASIC(UCharacterAttributeSet, StaminaMax);

	UFUNCTION()
	void OnRep_StaminaMax(const FGameplayAttributeData& OldValue)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UCharacterAttributeSet, StaminaMax, OldValue);
	}

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
