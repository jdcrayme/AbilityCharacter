// Copyright Epic Games, Inc. All Rights Reserved.

#include "CharacterAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(Tag_State_Dead, "State.Dead", "Tag applied when the character is dead.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(Tag_State_Stamina_Tired, "State.Stamina.Tired", "Tag applied when the character is tired.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Tag_State_Stamina_Exhausted, "State.Stamina.Exhausted", "Tag applied when the character is exhausted.");



UCharacterAttributeSet::UCharacterAttributeSet()
{
	Health = 100.0f;
	Stamina = 100.0f;
}

void UCharacterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetHealthAttribute())
		NewValue = FMath::Clamp(NewValue, 0.0f, GetHealthMax());
	
	if (Attribute == GetStaminaAttribute())
		NewValue = FMath::Clamp(NewValue, 0.0f, GetStaminaMax());
}

void UCharacterAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	// base values must respect the cap too, otherwise additive/multiplicative
	// base modifiers (or bad data) can push a base past its max and the current
	// value never visibly moves (stamina would never drain)
	if (Attribute == GetHealthAttribute())
		NewValue = FMath::Clamp(NewValue, 0.0f, GetHealthMax());

	if (Attribute == GetStaminaAttribute())
		NewValue = FMath::Clamp(NewValue, 0.0f, GetStaminaMax());
}
void UCharacterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
 
	// Check if the change was specifically to the Health attribute
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// Health was clamped to [0, HealthMax] in PreAttributeChange, so hitting
		// 0 means the character is out of health
		if (GetHealth() <= 0.0f)
		{
			if (!Data.Target.HasMatchingGameplayTag(Tag_State_Dead))
			{
				// Add the tag to eliminate further ability usage and trigger death logic
				Data.Target.AddLooseGameplayTag(Tag_State_Dead);
			}
		}
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		// tired: below half the stamina pool
		if (GetStamina() <= 50)
		{
			if (!Data.Target.HasMatchingGameplayTag(Tag_State_Stamina_Tired))
				Data.Target.AddLooseGameplayTag(Tag_State_Stamina_Tired);
		}
		else
		{
			if (Data.Target.HasMatchingGameplayTag(Tag_State_Stamina_Tired))
				Data.Target.RemoveLooseGameplayTag(Tag_State_Stamina_Tired);
		}

		// exhausted: nearly empty
		if (GetStamina() <= 15)
		{
			if (!Data.Target.HasMatchingGameplayTag(Tag_State_Stamina_Exhausted))
				Data.Target.AddLooseGameplayTag(Tag_State_Stamina_Exhausted);
		}
		else
		{
			if (Data.Target.HasMatchingGameplayTag(Tag_State_Stamina_Exhausted))
				Data.Target.RemoveLooseGameplayTag(Tag_State_Stamina_Exhausted);
		}
	}
}

void UCharacterAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UCharacterAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCharacterAttributeSet, HealthMax, COND_None, REPNOTIFY_Always);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UCharacterAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCharacterAttributeSet, StaminaMax, COND_None, REPNOTIFY_Always);
}
