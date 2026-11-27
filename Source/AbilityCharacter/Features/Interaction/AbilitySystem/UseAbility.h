// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Engine/TimerHandle.h"
#include "Core/AbilitySystem/PassiveAbilityBase.h"
#include "Features/Interaction/Interfaces/CombatUsable.h"
#include "Components/WidgetComponent.h"
#include "UseAbility.generated.h"

class UUserWidget;
class UAbilityTask_WaitGameplayEvent;
class ACombatUsePromptActor;

/**
 *  Passive use ability.
 *  When granted, it line traces ahead of the character for usable targets -
 *  actors implementing the CombatUsable interface, or actors hosting a
 *  component that implements it (e.g. UCombatActionVolumeComponent on a
 *  vehicle) - and shows a world-space use prompt widget along the ray while
 *  one is in range. Pressing the use input (the Event.Input.UsePressed
 *  gameplay event, forwarded by the character like any other input) calls the
 *  target's Use function.
 *  Intended to be subclassed as a Blueprint ability and added to a character's
 *  DefaultAbilities: the Blueprint picks the prompt widget class (any widget
 *  implementing the CombatUsePromptInterface) and tunes the trace and prompt
 *  settings. The built-in CombatUsePromptWidget works out of the box.
 *  The trace and the prompt only run for locally controlled players; AI ability
 *  instances stay idle.
 */
UCLASS()
class ABILITYCHARACTER_API UUseAbility : public UPassiveAbilityBase
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

	/** Trace channel the interaction ray casts on; usable targets must block it to be found */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Use")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	/**
	 *  Prompt widget spawned in world space while a usable target is in range.
	 *  Any widget class implementing the CombatUsePromptInterface works; the
	 *  built-in CombatUsePromptWidget is the default.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Use|Prompt")
	TSubclassOf<UUserWidget> PromptWidgetClass;

	/** Space the prompt renders in: World is a diegetic quad at the target, Screen is camera-facing with a constant pixel size */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Use|Prompt")
	EWidgetSpace PromptSpace = EWidgetSpace::World;

	/** Prompt size: world units in World space, pixels in Screen space */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Use|Prompt")
	FVector2D PromptDrawSize = FVector2D(140.0f, 48.0f);

	/** Where along the interaction ray the prompt sits: 0.5 (default) is midway between the ray source and the intersection point */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Use|Prompt", meta = (ClampMin = 0))
	float PromptDistanceRatio = 0.5f;

	/** How fast the prompt follows its target position; 0 snaps with every trace */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Use|Prompt", meta = (ClampMin = 0))
	float PromptSmoothingSpeed = 12.0f;

	/** Key name shown in the prompt; keep it in sync with the use input mapping */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Use")
	FText UseKeyLabel;

	/** Draws the interaction ray for debugging */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Use|Debug")
	bool bDebugTrace = false;

protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/** Line traces ahead of the character for a usable target and updates the prompt */
	void RunUseCheck();

	/** True while this ability instance should drive the trace and prompt: the local player's machine */
	bool IsLocalPlayerAvatar() const;

	/**
	 *  Finds the usable target for a trace hit: the hit actor itself when it
	 *  implements CombatUsable, otherwise the first of the actor's components
	 *  implementing it (e.g. an action volume component on a vehicle).
	 */
	TScriptInterface<ICombatUsable> ResolveUsable(const FHitResult& Hit) const;

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

	/** Spawns (once) and refreshes the world-space prompt for the current usable target */
	void UpdatePrompt(const FVector& ViewLocation, const FVector& HitLocation, bool bSnapToTarget);

	/** Hides the prompt without destroying it */
	void HidePrompt();

	/** Destroys the prompt actor and forgets it */
	void DestroyPrompt();

	/** Usable actor or component currently in range, if any */
	UPROPERTY()
	TScriptInterface<ICombatUsable> CurrentUsable;

	/** World-space prompt holder carrying the prompt widget */
	UPROPERTY()
	TObjectPtr<ACombatUsePromptActor> PromptActor;

	/** True while the prompt is shown */
	bool bPromptVisible = false;

	/** Periodic trace timer */
	FTimerHandle TraceTimerHandle;

	/** Task listening for use input events */
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> UseEventTask;
};
