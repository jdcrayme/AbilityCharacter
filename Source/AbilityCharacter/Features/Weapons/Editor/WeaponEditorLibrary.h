// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WeaponEditorLibrary.generated.h"

class UAnimMontage;
class UAnimNotify;
class UTextBlock;
class UUserWidget;

/**
 *  Small editor-only helpers for the weapon demo setup: the montage notify
 *  array is not exposed to Python in UE 5.8 (protected), so notify placement
 *  goes through this library instead. Not shipped in any runtime flow.
 */
UCLASS(Meta = (ScriptName = "WeaponEditor"))
class ABILITYCHARACTER_API UWeaponEditorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 *  Adds an instant notify of NotifyClass at TriggerTime on the montage's
	 *  first notify track, following the same construction steps the montage
	 *  editor's SAnimNotifyTrack::CreateNewNotify performs. EventTag is applied
	 *  to the notify instance when it is a UAnimNotify_SendGameplayEvent.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon Editor")
	static bool AddMontageNotify(UAnimMontage* Montage, TSubclassOf<UAnimNotify> NotifyClass, float TriggerTime, FGameplayTag EventTag);

	/** Returns "name|class|time" per notify, for verification from scripts */
	UFUNCTION(BlueprintPure, Category = "Weapon Editor")
	static FString ListMontageNotifies(const UAnimMontage* Montage);

	/** Sets a text block's text from a string (BP graph helper) */
	UFUNCTION(BlueprintCallable, Category = "Weapon Editor")
	static void SetWidgetText(UTextBlock* Text, const FString& Value);

	/** Creates a widget of WidgetClass and adds it to the owning player's viewport */
	UFUNCTION(BlueprintCallable, Category = "Weapon Editor")
	static UUserWidget* CreateWidgetOnScreen(APlayerController* PC, TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder = 10);
};
