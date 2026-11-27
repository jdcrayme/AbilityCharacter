// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AnimNotify_SendGameplayEvent.generated.h"

/**
 *  Anim notify that forwards a gameplay event to the mesh owner's ability
 *  system, so abilities can react to exact animation beats.
 *
 *  The event tag is a property, so one notify class covers every beat:
 *  - Event.AttackTrace  - run the melee sweep / ranged hitscan (same event the
 *                         dedicated AnimNotify_DoAttackTrace sends)
 *  - Event.Weapon.MagIn - the magazine seats: transfer the ammo
 *  - Event.Weapon.Attach - the hand reaches the weapon: attach it
 *  The notify instance travels with the event as OptionalObject, and the
 *  receiving ability can read this notify's BoneName (for trace origins, for
 *  example) exactly like CombatAbilityBase reads AttackBoneName today.
 *
 *  AnimNotify_DoAttackTrace / CheckCombo / CheckChargedAttack remain for the
 *  existing montages (their instances are baked into those assets); new
 *  content, including every weapon montage, should use this notify instead.
 */
UCLASS()
class ABILITYCHARACTER_API UAnimNotify_SendGameplayEvent : public UAnimNotify
{
	GENERATED_BODY()

public:

	/** Gameplay event sent to the mesh owner's ability system when the notify fires */
	UPROPERTY(EditAnywhere, Category = "Event")
	FGameplayTag EventTag;

	/** Optional bone or socket the event concerns: passed to the receiving ability through the notify instance (e.g. the trace origin bone of an attack sweep) */
	UPROPERTY(EditAnywhere, Category = "Event")
	FName BoneName;

	/** Send the gameplay event to the mesh owner */
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	/** Show the event tag in the notify's track label */
	virtual FString GetNotifyName_Implementation() const override;
};
