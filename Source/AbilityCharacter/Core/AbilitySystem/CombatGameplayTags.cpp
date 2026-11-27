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

// Gameplay event sent when the player walks into a combat arena
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_ArenaEntered, "Event.ArenaEntered");

// Tags that identify abilities; input bindings, Blueprints and the StateTree
// activate abilities through these tags
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Attack_Primary, "Ability.Attack.Primary");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Attack_Secondary, "Ability.Attack.Secondary");

UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Stance_Combat, "Ability.Stance.Combat");
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
