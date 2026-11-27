// Copyright Epic Games, Inc. All Rights Reserved.

#include "PassiveAbilityBase.h"

#include "AbilitySystemComponent.h"

UPassiveAbilityBase::UPassiveAbilityBase()
{
	// one instance per actor that runs for the owner's lifetime
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bRetriggerInstancedAbility = false;
}

void UPassiveAbilityBase::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	// start the ability as soon as it is granted; granting alone doesn't activate
	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid() &&
		ActorInfo->OwnerActor.IsValid() &&
		ActorInfo->OwnerActor->GetLocalRole() == ROLE_Authority &&
		!Spec.IsActive())
	{
		ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle, false);
	}
}
