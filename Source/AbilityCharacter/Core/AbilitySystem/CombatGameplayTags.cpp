// Copyright Epic Games, Inc. All Rights Reserved.

#include "CombatGameplayTags.h"

// Events fired by montage notifies (AnimNotify_DoAttackTrace and the generic
// AnimNotify_SendGameplayEvent). The notify instance is passed in OptionalObject
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_AttackTrace, "Event.AttackTrace");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_CheckCombo, "Event.CheckCombo");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_CheckChargedAttack, "Event.CheckChargedAttack");

// SetByCaller magnitude keys used by damage and cost effects
UE_DEFINE_GAMEPLAY_TAG(TAG_DataDamage, "Data.Damage");
UE_DEFINE_GAMEPLAY_TAG(TAG_Data_StaminaCost, "Data.StaminaCost");

// Input-derived gameplay events: input actions are forwarded into the ability
// system as these events; abilities trigger on them (AbilityTriggers) or listen
// for them with WaitGameplayEvent
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_InputPrimaryPressed, "Event.Input.PrimaryPressed");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_InputSecondaryPressed, "Event.Input.SecondaryPressed");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_InputSecondaryReleased, "Event.Input.SecondaryReleased");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_InputUsePressed, "Event.Input.UsePressed");

// Weapon input events: the weapon abilities trigger on the presses (AbilityTriggers) and
// listen for the releases (WaitGameplayEvent), exactly like the charged attack does
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_InputFirePressed, "Event.Input.FirePressed");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_InputReloadPressed, "Event.Input.ReloadPressed");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_InputAimPressed, "Event.Input.AimPressed");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_InputAimReleased, "Event.Input.AimReleased");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_InputEquipPressed, "Event.Input.EquipPressed");

// Gameplay event sent when the player walks into a combat arena
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_ArenaEntered, "Event.ArenaEntered");

// Tags that identify abilities; input bindings, Blueprints and the StateTree
// activate abilities through these tags
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Attack_Primary, "Ability.Attack.Primary");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Attack_Secondary, "Ability.Attack.Secondary");

UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Stance_Combat, "Ability.Stance.Combat");

// Weapon ability identity tags
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Weapon_Fire, "Ability.Weapon.Fire");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Weapon_Reload, "Ability.Weapon.Reload");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Weapon_Aim, "Ability.Weapon.Aim");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Weapon_Equip, "Ability.Weapon.Equip");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Stance_Neutral, "Ability.Stance.Neutral");

// Gameplay cue executed on damageable targets after a successful hit. Carries the impact point, normal and knockback impulse in its effect context
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayCue_Damage, "GameplayCue.Damage");

// State tags. Stances are granted through abilities (see USetStanceAbility) and drive anim sets, movement and ability gating.
// State.Stance doubles as the replicated tag slot key: the stance ability applies all stance tags through that slot
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Stance, "State.Stance");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Stance_Combat, "State.Stance.Combat");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Stance_Walk, "State.Stance.Walk");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Stance_Run, "State.Stance.Run");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Stance_Sneak, "State.Stance.Sneak");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Stance_Crouch, "State.Stance.Crouch");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Stance_Prone, "State.Stance.Prone");

// Weapon state tags. State.Weapon is both a real tag and the replicated slot key the
// weapon equipment component applies state tags through (ApplyGrantedTags), mirroring
// how the stance abilities use the State.Stance slot
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Weapon, "State.Weapon");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Weapon_Equipped, "State.Weapon.Equipped");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Weapon_Drawn, "State.Weapon.Drawn");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Weapon_Reloading, "State.Weapon.Reloading");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Weapon_Swapping, "State.Weapon.Swapping");

UE_DEFINE_GAMEPLAY_TAG(TAG_State_Aiming, "State.Aiming");

// Weapon montage beats, fired by AnimNotify_SendGameplayEvent instances placed in
// equip/reload montages
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Weapon_Swap, "Event.Weapon.Swap");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Weapon_MagIn, "Event.Weapon.MagIn");

// SetByCaller key for weapon cooldown durations (the fire interval)
UE_DEFINE_GAMEPLAY_TAG(TAG_Data_Cooldown, "Data.Cooldown");

// Shared ammo types: weapons declare the type they consume, inventory entries count it.
// More types go in DefaultGameplayTags.ini as content needs them
UE_DEFINE_GAMEPLAY_TAG(TAG_Ammo_Rifle, "Ammo.Rifle");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ammo_Shotgun, "Ammo.Shotgun");
