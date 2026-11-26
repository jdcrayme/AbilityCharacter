// Copyright Epic Games, Inc. All Rights Reserved.


#include "AnimNotify_DoAttackTrace.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CombatGameplayTags.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotify_DoAttackTrace::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	// forward the trace request to the running attack ability through a gameplay event.
	// the notify instance travels with the event so the ability knows which bone to trace from
	FGameplayEventData EventData;
	EventData.EventTag = TAG_Event_AttackTrace;
	EventData.Instigator = Owner;
	EventData.OptionalObject = this;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, TAG_Event_AttackTrace, EventData);
}

FString UAnimNotify_DoAttackTrace::GetNotifyName_Implementation() const
{
	return FString("Do Attack Trace");
}
