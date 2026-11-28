// Copyright Epic Games, Inc. All Rights Reserved.

#include "Features/Weapons/Editor/WeaponEditorLibrary.h"

#include "Animation/AnimMontage.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Features/Combat/Animation/AnimNotify_SendGameplayEvent.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"

void UWeaponEditorLibrary::SetWidgetText(UTextBlock* Text, const FString& Value)
{
	if (Text)
	{
		Text->SetText(FText::FromString(Value));
	}
}

UUserWidget* UWeaponEditorLibrary::CreateWidgetOnScreen(APlayerController* PC, TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder)
{
	if (!PC || !PC->GetWorld() || !*WidgetClass)
	{
		return nullptr;
	}

	UUserWidget* Widget = CreateWidget<UUserWidget>(PC->GetWorld(), WidgetClass);
	if (Widget)
	{
		Widget->AddToViewport(ZOrder);
	}
	return Widget;
}

bool UWeaponEditorLibrary::AddMontageNotify(UAnimMontage* Montage, TSubclassOf<UAnimNotify> NotifyClass, float TriggerTime, FGameplayTag EventTag)
{
	if (!Montage || !NotifyClass)
	{
		return false;
	}
	if (TriggerTime < 0.0f)
	{
		TriggerTime = 0.0f;
	}

	Montage->Modify();

	const int32 NewNotifyIndex = Montage->Notifies.Add(FAnimNotifyEvent());
	FAnimNotifyEvent& NewEvent = Montage->Notifies[NewNotifyIndex];
	NewEvent.NotifyName = NotifyClass->GetFName();
	NewEvent.Guid = FGuid::NewGuid();
	NewEvent.Link(Montage, TriggerTime);
	NewEvent.TriggerTimeOffset = -SMALL_NUMBER;
	NewEvent.TrackIndex = 0;

	UObject* NotifyInstance = NewObject<UObject>(Montage, NotifyClass, NAME_None, RF_Transactional);
	NewEvent.NotifyStateClass = Cast<UAnimNotifyState>(NotifyInstance);
	NewEvent.Notify = Cast<UAnimNotify>(NotifyInstance);
	if (NewEvent.Notify)
	{
		NewEvent.TriggerWeightThreshold = NewEvent.Notify->GetDefaultTriggerWeightThreshold();
	}
	if (UAnimNotify_SendGameplayEvent* SendEvent = Cast<UAnimNotify_SendGameplayEvent>(NotifyInstance))
	{
		SendEvent->EventTag = EventTag;
	}

	Montage->PostEditChange();
	Montage->MarkPackageDirty();
	return true;
}

FString UWeaponEditorLibrary::ListMontageNotifies(const UAnimMontage* Montage)
{
	if (!Montage)
	{
		return TEXT("");
	}

	TArray<FString> Lines;
	for (const FAnimNotifyEvent& Event : Montage->Notifies)
	{
		Lines.Add(FString::Printf(TEXT("%s|%s|%.3f"),
			*Event.NotifyName.ToString(),
			Event.Notify ? *Event.Notify->GetClass()->GetName() : TEXT("None"),
			Event.GetTriggerTime()));
	}
	return FString::Join(Lines, TEXT("\n"));
}