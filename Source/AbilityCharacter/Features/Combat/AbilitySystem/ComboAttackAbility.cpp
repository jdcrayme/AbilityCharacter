// Copyright Epic Games, Inc. All Rights Reserved.

#include "ComboAttackAbility.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Core/AbilitySystem/CombatGameplayTags.h"
#include "Animation/AnimMontage.h"
#include "UObject/ConstructorHelpers.h"

UComboAttackAbility::UComboAttackAbility()
{
	// generic attack tag: input bindings, Blueprints and the StateTree all activate
	// this ability through Ability.Attack.Primary
	SetAssetTags(FGameplayTagContainer(TAG_Ability_Attack_Primary));

	// the ability activates from the primary attack input event; further presses are
	// buffered as events while the combo string is running
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = TAG_Event_InputPrimaryPressed;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

	// the combo montage drives the attack through its gameplay event notifies
	static ConstructorHelpers::FObjectFinder<UAnimMontage> ComboMontageFinder(TEXT("/Game/Variant_Combat/Anims/AM_ComboAttack"));
	if (ComboMontageFinder.Succeeded())
	{
		MontageToPlay = ComboMontageFinder.Object;
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

	ComboSectionNames = { TEXT("Melee01"), TEXT("Melee02"), TEXT("Melee03") };
}

void UComboAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ComboCount = 0;

	// the input event that activated us counts as the first press
	if (UsesInputBuffering())
	{
		LastAttackInputTime = GetWorld()->GetTimeSeconds();

		InputEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TAG_Event_InputPrimaryPressed);
		InputEventTask->EventReceived.AddDynamic(this, &UComboAttackAbility::OnAttackInputEvent);
		InputEventTask->ReadyForActivation();
	}

	ComboCheckTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TAG_Event_CheckCombo);
	ComboCheckTask->EventReceived.AddDynamic(this, &UComboAttackAbility::OnComboCheckEvent);
	ComboCheckTask->ReadyForActivation();
}

bool UComboAttackAbility::WantsNextComboSection() const
{
	// continue the string if the player pressed attack again recently enough
	if (!GetWorld()) return false;
	return (GetWorld()->GetTimeSeconds() - LastAttackInputTime) <= ComboInputCacheTimeTolerance;
}

bool UComboAttackAbility::UsesInputBuffering() const
{
	return true;
}

void UComboAttackAbility::OnComboCheckEvent(FGameplayEventData EventData)
{
	const int32 NextSection = ComboCount + 1;

	if (NextSection >= ComboSectionNames.Num())
	{
		UE_LOG(LogTemp, Verbose, TEXT("[Combo] %s: combo string finished at section %d"), *GetName(), ComboCount);
		return;
	}

	if (!WantsNextComboSection())
	{
		UE_LOG(LogTemp, Verbose, TEXT("[Combo] %s: no buffered input, stopping at section %d"), *GetName(), ComboCount);
		return;
	}

	// consume the buffered input so the same press can't advance twice
	LastAttackInputTime = -1000.0f;
	ComboCount = NextSection;

	UE_LOG(LogTemp, Verbose, TEXT("[Combo] %s: chaining to section %s"), *GetName(), *ComboSectionNames[ComboCount].ToString());
	MontageJumpToSection(ComboSectionNames[ComboCount]);
}

void UComboAttackAbility::OnAttackInputEvent(FGameplayEventData EventData)
{
	LastAttackInputTime = GetWorld()->GetTimeSeconds();
}
