// Copyright Epic Games, Inc. All Rights Reserved.

#include "SetStanceAbility.h"

#include "AbilitySystemComponent.h"

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

	// apply the new stance tags
	for (const FGameplayTag& TagToAdd : StanceTagsToAdd)
	{
		ASC->AddLooseGameplayTag(TagToAdd);
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
