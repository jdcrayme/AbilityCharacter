// Copyright Epic Games, Inc. All Rights Reserved.


#include "AnimNotify_CheckCombo.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Core/AbilitySystem/CombatGameplayTags.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotify_CheckCombo::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	// ask the running combo attack ability whether to continue the string
	FGameplayEventData EventData;
	EventData.EventTag = TAG_Event_CheckCombo;
	EventData.Instigator = Owner;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, TAG_Event_CheckCombo, EventData);
}

FString UAnimNotify_CheckCombo::GetNotifyName_Implementation() const
{
	return FString("Check Combo String");
}
