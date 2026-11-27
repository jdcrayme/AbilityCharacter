// Copyright Epic Games, Inc. All Rights Reserved.

#include "HideByTagComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

UHideByTagComponent::UHideByTagComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHideByTagComponent::BeginPlay()
{
	Super::BeginPlay();

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (ASC && WatchingTag.IsValid())
	{
		// this delegate triggers when the tag OR its descendants change
		ASC->RegisterGameplayTagEvent(WatchingTag, EGameplayTagEventType::AnyCountChange)
			.AddUObject(this, &UHideByTagComponent::OnTagChanged);

		// initial check
		OnTagChanged(WatchingTag, ASC->GetTagCount(WatchingTag));
	}
}

void UHideByTagComponent::OnTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	SetVisibility(NewCount > 0, true);
}
