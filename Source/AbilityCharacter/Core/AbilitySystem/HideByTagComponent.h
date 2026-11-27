// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameplayTagContainer.h"
#include "HideByTagComponent.generated.h"

/**
 *  Scene component that hides itself while its owner carries a gameplay tag.
 *  Watches the owner's ability system component for WatchingTag (the tag or
 *  any of its children) and toggles its own visibility, propagating to
 *  attached children.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ABILITYCHARACTER_API UHideByTagComponent : public USceneComponent
{
	GENERATED_BODY()

public:

	/** Constructor */
	UHideByTagComponent();

protected:

	virtual void BeginPlay() override;

	/** The tag to watch (e.g. "State.Alerted"); the component hides while the owner carries it */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
	FGameplayTag WatchingTag;

	/** Callback when the watched tag's count changes */
	void OnTagChanged(const FGameplayTag Tag, int32 NewCount);
};
