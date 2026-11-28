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

/** Gameplay event sent when the weapon fire input is pressed (the weapon fire ability triggers on it) */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_InputFirePressed);

/** Gameplay event sent when the weapon reload input is pressed */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_InputReloadPressed);

/** Gameplay event sent when the aim input is pressed (hold-style: pair with AimReleased) */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_InputAimPressed);

/** Gameplay event sent when the aim input is released; ends the running aim ability */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_InputAimReleased);

/** Gameplay event sent when the weapon equip input is pressed */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_InputEquipPressed);

/** Gameplay event sent when the player walks into a combat arena */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_ArenaEntered);

/** Tag that identifies the primary attack ability (combo attack). Input, Blueprints and the StateTree activate abilities through this tag */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Attack_Primary);

/** Tag that identifies the secondary attack ability (charged attack) */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Attack_Secondary);

/** Tag that identifies the combat stance ability */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Stance_Combat);

/** Tags that identify the weapon abilities; input bindings, Blueprints and AI activate them through these tags */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Weapon_Fire);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Weapon_Reload);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Weapon_Aim);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Weapon_Equip);

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

/**
 *  State.Weapon doubles as the replicated tag slot key for equipment state tags, exactly
 *  like State.Stance does for stances: the weapon equipment component applies all weapon
 *  state tags through that slot (see UCombatAbilitySystemComponent::ApplyGrantedTags), so
 *  swapping weapon state removes whatever the previous state granted.
 */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Weapon);

/** Granted while any weapon is equipped (slot: State.Weapon) */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Weapon_Equipped);

/** Granted while the equipped weapon is drawn and ready (slot: State.Weapon). Fire, reload and aim require it */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Weapon_Drawn);

/** Granted while a magazine reload is running (slot: State.Weapon). Blocks fire and equip */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Weapon_Reloading);

/** Granted while a draw/holster swap is running (slot: State.Weapon). Blocks fire and reload */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Weapon_Swapping);

/** Granted while the aim ability is running (slot of its own, cleared when aiming ends).
 *  Animation blueprints drive aim poses off this tag; fire abilities can tighten spread with it */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Aiming);

/** Events fired by weapon montages through AnimNotify_SendGameplayEvent: Swap is the frame
 *  the hand reaches the weapon (the attach/unlink moment), MagIn is the frame the magazine
 *  seats (the reload ability transfers ammo on it) */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Weapon_Swap);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Weapon_MagIn);

/** SetByCaller magnitude key used by weapon cooldown effects ("Data.Cooldown"), in seconds */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Data_Cooldown);

/** Shared ammo type tags. Ammo types are the type system for ammunition: weapons declare
 *  the type they consume and inventory entries store counts keyed by the same tag, so
 *  anything that can AddEntry(Ammo.X, N) can feed a weapon that consumes Ammo.X. Further
 *  types are declared in DefaultGameplayTags.ini as content requires them */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ammo_Rifle);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ammo_Shotgun);
