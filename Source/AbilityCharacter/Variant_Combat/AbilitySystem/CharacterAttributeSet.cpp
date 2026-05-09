#include "CharacterAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(Tag_State_Dead, "State.Dead", "Tag applied when the character is dead.");



UCharacterAttributeSet::UCharacterAttributeSet()
{
	Health = 100.f;
}

void UCharacterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetHealthMax());
	}
}
void UCharacterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
 
	// Check if the change was specifically to the Health attribute
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// Because we clamped in PreAttributeChange, we check if it is exactly 0
		if (GetHealth() <= 0.0f)
		{
 
			if (!Data.Target.HasMatchingGameplayTag(Tag_State_Dead))
			{
				// Add the tag to eliminate further ability usage or trigger death logic
				Data.Target.AddLooseGameplayTag(Tag_State_Dead);
                
				// Optional: You can also call a function on the Actor to handle elimination
				// AActor* TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
			}
		}
	}
}

void UCharacterAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UCharacterAttributeSet, Health, COND_None, REPNOTIFY_Always);
}
