#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "HideByTagComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UHideByTagComponent : public USceneComponent
{
	GENERATED_BODY()
 
public:
	UHideByTagComponent();
 
protected:
	virtual void BeginPlay() override;
 
	// The tag to watch (e.g., "State.Alerted")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
	FGameplayTag WatchingTag;
  
	// Callback when tag count changes
	void OnTagChanged(const FGameplayTag Tag, int32 NewCount);
 
};