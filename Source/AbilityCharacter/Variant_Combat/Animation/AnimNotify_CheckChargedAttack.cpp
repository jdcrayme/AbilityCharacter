// Copyright Epic Games, Inc. All Rights Reserved.


#include "AnimNotify_CheckChargedAttack.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "CombatGameplayTags.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotify_CheckChargedAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	// ask the running charged attack ability whether to keep charging or release
	FGameplayEventData EventData;
	EventData.EventTag = TAG_Event_CheckChargedAttack;
	EventData.Instigator = Owner;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, TAG_Event_CheckChargedAttack, EventData);
}

FString UAnimNotify_CheckChargedAttack::GetNotifyName_Implementation() const
{
	return FString("Check Charged Attack");
}
