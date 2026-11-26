// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

/**
 *  Native gameplay tags for the combat demo.
 *  All combat flow is routed through the GameplayAbility system:
 *  - Attacks are tagged Ability.Attack.Primary / Ability.Attack.Secondary so
 *    input bindings, Blueprints and the StateTree can activate them uniformly.
 *  - Montage notifies fire gameplay events consumed by attack abilities.
 *  - Player input is forwarded as gameplay events.
 *  - Damage feedback is delivered through the GameplayCue.Damage cue.
 *  - Stances (State.Stance.*) are granted by stance abilities and gate combat
 *    behaviour and anim sets.
 */

/** Gameplay event fired by the DoAttackTrace montage notify. The notify instance is passed in OptionalObject */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_AttackTrace);

/** Gameplay event fired by the CheckCombo montage notify */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_CheckCombo);

/** Gameplay event fired by the CheckChargedAttack montage notify */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_CheckChargedAttack);

/** SetByCaller magnitude key used by damage effects ("Data.Damage") */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_DataDamage);

/** SetByCaller magnitude key used by stamina cost effects ("Data.StaminaCost") */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Data_StaminaCost);

/** Gameplay event sent when the primary attack input is pressed. Activates and buffers the primary attack ability */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_InputPrimaryPressed);

/** Gameplay event sent when the secondary attack input is pressed */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_InputSecondaryPressed);

/** Gameplay event sent when the secondary attack input is released */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_InputSecondaryReleased);

/** Gameplay event sent when the use input is pressed; consumed by the passive use ability */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_InputUsePressed);

/** Gameplay event sent when the player walks into a combat arena */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_ArenaEntered);

/** Tag that identifies the primary attack ability (combo attack). Input, Blueprints and the StateTree activate abilities through this tag */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Attack_Primary);

/** Tag that identifies the secondary attack ability (charged attack) */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Attack_Secondary);

/** Tag that identifies the combat stance ability */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Stance_Combat);

/** Tag that identifies the neutral stance ability */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Stance_Neutral);

/** Gameplay cue executed on damageable targets after a successful hit. Carries the impact point, normal and knockback impulse in its effect context */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GameplayCue_Damage);

/** Stance tags. Stances are granted through abilities (see USetStanceAbility) and drive anim sets, movement and ability gating */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Stance);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Stance_Combat);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Stance_Walk);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Stance_Run);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Stance_Sneak);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Stance_Crouch);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Stance_Prone);
