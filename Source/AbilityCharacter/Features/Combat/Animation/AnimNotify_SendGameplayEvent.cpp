// Copyright Epic Games, Inc. All Rights Reserved.

#include "AnimNotify_SendGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotify_SendGameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	// untagged notifies have nothing to send; leave them on the track as reminders
	if (!EventTag.IsValid())
	{
		return;
	}

	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!Owner) return;

	// forward the animation beat to the ability system as a gameplay event.
	// the notify instance travels with the event so the receiving ability can
	// read this notify's BoneName (trace origins and similar payloads)
	FGameplayEventData EventData;
	EventData.EventTag = EventTag;
	EventData.Instigator = Owner;
	EventData.OptionalObject = this;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventTag, EventData);
}

FString UAnimNotify_SendGameplayEvent::GetNotifyName_Implementation() const
{
	return EventTag.IsValid()
		? FString::Printf(TEXT("Event: %s"), *EventTag.ToString())
		: TEXT("Event: (unset)");
}
