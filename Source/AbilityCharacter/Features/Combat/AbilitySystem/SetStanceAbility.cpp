// Copyright Epic Games, Inc. All Rights Reserved.

#include "SetStanceAbility.h"

#include "AbilitySystemComponent.h"
#include "Core/AbilitySystem/CombatAbilitySystemComponent.h"
#include "Core/AbilitySystem/CombatGameplayTags.h"

USetStanceAbility::USetStanceAbility()
{
	// re-activating a stance ability while a stance is already set swaps the stance
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bRetriggerInstancedAbility = true;
}

void USetStanceAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// drop the previous stance tags
	for (const FGameplayTag& TagToRemove : StanceTagsToRemove)
	{
		ASC->RemoveLooseGameplayTag(TagToRemove);
	}

	// revoke abilities that belong to other stances
	for (const TSubclassOf<UGameplayAbility>& AbilityClass : AbilitiesToRevoke)
	{
		if (AbilityClass)
		{
			RevokeAbilitiesOfClass(AbilityClass);
		}
	}

	// apply the new stance tags through the replicated State.Stance slot: one effect
	// grants all of them and replaces whatever the previous stance granted. Effect-
	// granted tags replicate to every client (loose tags never do), so animation
	// blueprints can drive poses off State.Stance.* on machines that don't run the
	// stance logic
	if (UCombatAbilitySystemComponent* CombatASC = Cast<UCombatAbilitySystemComponent>(ASC))
	{
		if (StanceTagsToAdd.IsEmpty())
		{
			CombatASC->ClearGrantedTags(TAG_State_Stance);
		}
		else
		{
			CombatASC->ApplyGrantedTags(TAG_State_Stance, StanceTagsToAdd);
		}
	}
	else
	{
		// fall back to loose tags for non-combat ability system components
		for (const FGameplayTag& TagToAdd : StanceTagsToAdd)
		{
			ASC->AddLooseGameplayTag(TagToAdd);
		}
	}

	// grant the abilities this stance provides (skip ones already granted so
	// re-triggering the stance doesn't churn running abilities)
	for (const TSubclassOf<UGameplayAbility>& AbilityClass : AbilitiesToGrant)
	{
		if (AbilityClass && !HasAbilityOfClass(AbilityClass))
		{
			ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, GetAbilityLevel(), INDEX_NONE, GetAvatarActorFromActorInfo()));
		}
	}

	// remove any stance effect applied by a previous activation
	if (StanceEffectHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(StanceEffectHandle);
		StanceEffectHandle = FActiveGameplayEffectHandle();
	}

	if (StanceEffect)
	{
		const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(StanceEffect, GetAbilityLevel(), ASC->MakeEffectContext());
		if (SpecHandle.IsValid())
		{
			StanceEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void USetStanceAbility::RevokeAbilitiesOfClass(TSubclassOf<UGameplayAbility> AbilityClass)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC || !AbilityClass) return;

	// collect the specs first: clearing while iterating the container is unsafe
	TArray<FGameplayAbilitySpecHandle> HandlesToClear;
	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.Ability && Spec.Ability->GetClass() == AbilityClass)
		{
			HandlesToClear.Add(Spec.Handle);
		}
	}

	for (const FGameplayAbilitySpecHandle& Handle : HandlesToClear)
	{
		ASC->ClearAbility(Handle);
	}
}

bool USetStanceAbility::HasAbilityOfClass(TSubclassOf<UGameplayAbility> AbilityClass) const
{
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC || !AbilityClass) return false;

	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.Ability && Spec.Ability->GetClass() == AbilityClass)
		{
			return true;
		}
	}

	return false;
}
