// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Engine/TimerHandle.h"
#include "Abilities/GameplayAbility.h"
#include "UseAbility.generated.h"

class UCombatUsePromptWidget;
class UAbilityTask_WaitGameplayEvent;

/**
 *  Passive use ability.
 *  When granted, it line traces ahead of the character for actors implementing
 *  the CombatUsable interface (e.g. action volumes) and shows a "use" prompt
 *  widget while one is in range. Pressing the use input (the
 *  Combat.Input.UsePressed gameplay event, forwarded by the character just like
 *  the attack inputs) calls the usable actor's Use function.
 *  The trace and the prompt only run for locally controlled players; AI ability
 *  instances stay idle.
 */
UCLASS()
class ABILITYCHARACTER_API UUseAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:

	/** Constructor */
	UUseAbility();

	/** How far ahead of the character the interaction ray reaches */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Use", meta = (ClampMin = 0, Units = "cm"))
	float TraceDistance = 350.0f;

	/** How often the interaction ray casts */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Use", meta = (ClampMin = 0.01, Units = "s"))
	float TraceInterval = 0.1f;

	/** Trace channel the interaction ray casts on; usable actors must block it to be found */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Use")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	/** Prompt widget shown while a usable actor is in range */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Use")
	TSubclassOf<UCombatUsePromptWidget> PromptWidgetClass;

	/** Key name shown in the prompt; keep it in sync with the use input mapping */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Use")
	FText UseKeyLabel;

	/** Draws the interaction ray for debugging */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Use|Debug")
	bool bDebugTrace = false;

protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/** Passive ability: start running as soon as it is granted */
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	/** Line traces ahead of the character for a usable actor and updates the prompt */
	void RunUseCheck();

	/** True while this ability instance should drive the trace and prompt: the local player's machine */
	bool IsLocalPlayerAvatar() const;

private:

	/** Called by the periodic trace timer */
	UFUNCTION()
	void OnTraceTimer();

	/** Called when the use input gameplay event arrives */
	UFUNCTION()
	void OnUsePressed(FGameplayEventData EventData);

	/** Called when the avatar actor is destroyed (e.g. on respawn) */
	UFUNCTION()
	void OnAvatarDestroyed(AActor* DestroyedActor);

	/** Shows or refreshes the prompt for the provided usable actor */
	void ShowPrompt(AActor* UsableActor);

	/** Collapses the prompt without destroying it */
	void HidePrompt();

	/** Removes the prompt from the viewport and forgets it */
	void DestroyPrompt();

	/** Usable actor currently in range, if any */
	UPROPERTY()
	TObjectPtr<AActor> CurrentUsableActor;

	/** Prompt widget instance */
	UPROPERTY()
	TObjectPtr<UCombatUsePromptWidget> PromptWidget;

	/** True while the prompt is on screen */
	bool bPromptVisible = false;

	/** Periodic trace timer */
	FTimerHandle TraceTimerHandle;

	/** Task listening for use input events */
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> UseEventTask;
};
