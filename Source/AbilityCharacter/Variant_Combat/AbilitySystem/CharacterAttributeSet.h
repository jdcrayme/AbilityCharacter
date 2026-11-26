#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "NativeGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "CharacterAttributeSet.generated.h"

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
		GAMEPLAYATTRIBUTE_REPNOTIFY(UCharacterAttributeSet, HealthMax, OldValue);
	}
	
	void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue);
	void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const;
 	void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};