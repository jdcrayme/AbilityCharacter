
#include "HideByTagComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

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
		// This delegate triggers when the tag OR its descendants change
		ASC->RegisterGameplayTagEvent(WatchingTag, EGameplayTagEventType::AnyCountChange)
		   .AddUObject(this, &UHideByTagComponent::OnTagChanged);
 
		// Initial check
		OnTagChanged(WatchingTag, ASC->GetTagCount(WatchingTag));
	}
}

void UHideByTagComponent::OnTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	SetVisibility(NewCount >0, true);
}
