// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChargedAttackAbility.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Core/AbilitySystem/CombatGameplayTags.h"
#include "Animation/AnimMontage.h"
#include "UObject/ConstructorHelpers.h"

UChargedAttackAbility::UChargedAttackAbility()
{
	// generic attack tag: input bindings, Blueprints and the StateTree all activate
	// this ability through Ability.Attack.Secondary
	SetAssetTags(FGameplayTagContainer(TAG_Ability_Attack_Secondary));

	// the ability activates from the secondary attack input press event
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = TAG_Event_InputSecondaryPressed;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

	// the charged attack montage drives the charge loop through its gameplay event notifies
	static ConstructorHelpers::FObjectFinder<UAnimMontage> ChargedMontageFinder(TEXT("/Game/Variant_Combat/Anims/AM_ChargedAttack"));
	if (ChargedMontageFinder.Succeeded())
	{
		MontageToPlay = ChargedMontageFinder.Object;
	}

	// damage applied attacker-side to every target the trace sweeps hit
	static ConstructorHelpers::FClassFinder<UGameplayEffect> DamageEffectFinder(TEXT("/Game/Variant_Combat/Effect/GE_DamageHealth"));
	if (DamageEffectFinder.Succeeded())
	{
		EffectToApply = DamageEffectFinder.Class;
	}

	// player tuning: matches the previous combat character configuration
	AttackDamage = 26.0f;
	KnockbackImpulse = 250.0f;
	LaunchImpulse = 300.0f;
	MeleeTraceRadius = 75.0f;

	// a charged release is a heavier commitment than a combo hit
	StaminaCost = 25.0f;

	// section names used by the bundled charge attack montage
	ChargeLoopSection = TEXT("Charge");
	ChargeAttackSection = TEXT("Attack");
}

void UChargedAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	bHasLoopedCharge = false;
	bChargeReleased = false;

	ChargeCheckTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TAG_Event_CheckChargedAttack);
	ChargeCheckTask->EventReceived.AddDynamic(this, &UChargedAttackAbility::OnChargeCheckEvent);
	ChargeCheckTask->ReadyForActivation();

	ChargeReleaseTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TAG_Event_InputSecondaryReleased);
	ChargeReleaseTask->EventReceived.AddDynamic(this, &UChargedAttackAbility::OnChargeReleasedEvent);
	ChargeReleaseTask->ReadyForActivation();
}

bool UChargedAttackAbility::ShouldReleaseCharge()
{
	// players release the charge by letting go of the input
	return bChargeReleased;
}

void UChargedAttackAbility::OnChargeCheckEvent(FGameplayEventData EventData)
{
	bHasLoopedCharge = true;

	const bool bRelease = ShouldReleaseCharge();
	UE_LOG(LogTemp, Verbose, TEXT("[Charged] %s: charge check, release=%d"), *GetName(), bRelease);
	MontageJumpToSection(bRelease ? ChargeAttackSection : ChargeLoopSection);
}

void UChargedAttackAbility::OnChargeReleasedEvent(FGameplayEventData EventData)
{
	bChargeReleased = true;

	// if we've charged at least one loop, release the attack right away
	if (bHasLoopedCharge)
	{
		MontageJumpToSection(ChargeAttackSection);
	}
}
