// Copyright Epic Games, Inc. All Rights Reserved.

#include "CombatAbilityBase.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "Core/Characters/CharacterBase.h"
#include "Core/AbilitySystem/CombatEffectContext.h"
#include "Core/AbilitySystem/CombatGameplayTags.h"
#include "Features/Combat/AbilitySystem/CombatStaminaCostEffect.h"
#include "Core/Interfaces/CombatDamageable.h"
#include "Features/Combat/Animation/AnimNotify_DoAttackTrace.h"

UCombatAbilityBase::UCombatAbilityBase()
{
	// one active instance per actor: re-triggering while attacking is rejected so
	// buffered input events reach the running instance instead of starting a parallel one
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bRetriggerInstancedAbility = false;

	// every attack drains stamina through the shared SetByCaller cost effect
	CostGameplayEffectClass = UCombatStaminaCostEffect::StaticClass();
}

void UCombatAbilityBase::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (StaminaCost <= 0.0f || !CostGameplayEffectClass)
	{
		Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC) return;

	// the cost amount travels through the Data.StaminaCost SetByCaller magnitude
	const FGameplayEffectSpecHandle CostSpecHandle = ASC->MakeOutgoingSpec(CostGameplayEffectClass, GetAbilityLevel(), ASC->MakeEffectContext());
	if (CostSpecHandle.IsValid())
	{
		CostSpecHandle.Data->SetSetByCallerMagnitude(TAG_Data_StaminaCost, -StaminaCost);
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CostSpecHandle);
	}
}

void UCombatAbilityBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* Avatar = GetAvatarActorFromActorInfo();
	UE_LOG(LogTemp, Warning, TEXT("[Combat] %s activating on %s (montage=%s, effect=%s)"),
		*GetName(), *GetNameSafe(Avatar),
		MontageToPlay ? *MontageToPlay->GetName() : TEXT("NONE"),
		EffectToApply ? *EffectToApply->GetName() : TEXT("NONE"));
	if (!Avatar)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// warn nearby damageable actors so they can react to the incoming attack
	NotifyEnemiesOfIncomingAttack();

	if (MontageToPlay)
	{
		MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, MontageToPlay);
		MontageTask->OnCompleted.AddDynamic(this, &UCombatAbilityBase::OnMontageFinished);
		MontageTask->OnInterrupted.AddDynamic(this, &UCombatAbilityBase::OnMontageFinished);
		MontageTask->OnCancelled.AddDynamic(this, &UCombatAbilityBase::OnMontageFinished);
	}

	// hit sweeps are driven by the montage notifies through gameplay events
	TraceEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TAG_Event_AttackTrace);
	TraceEventTask->EventReceived.AddDynamic(this, &UCombatAbilityBase::OnAttackTraceEvent);

	if (MontageTask)
	{
		MontageTask->ReadyForActivation();
		TraceEventTask->ReadyForActivation();
	}
	else
	{
		// no montage to drive the attack: end right away instead of hanging active forever
		TraceEventTask->ReadyForActivation();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UCombatAbilityBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	// let the avatar know the attack finished; AI characters forward this to their StateTree
	if (ACharacterBase* Character = Cast<ACharacterBase>(GetAvatarActorFromActorInfo()))
	{
		Character->OnAttackAbilityEnded.Broadcast();
	}
}

void UCombatAbilityBase::NotifyEnemiesOfIncomingAttack()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar) return;

	// sweep for objects in front of the character about to be hit by the attack
	TArray<FHitResult> OutHits;

	const FVector TraceStart = Avatar->GetActorLocation();
	const FVector TraceEnd = TraceStart + (Avatar->GetActorForwardVector() * DangerTraceDistance);

	// only warn pawns
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(DangerTraceRadius);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Avatar);

	if (GetWorld()->SweepMultiByObjectType(OutHits, TraceStart, TraceEnd, FQuat::Identity, ObjectParams, CollisionShape, QueryParams))
	{
		for (const FHitResult& CurrentHit : OutHits)
		{
			if (ICombatDamageable* Damageable = Cast<ICombatDamageable>(CurrentHit.GetActor()))
			{
				Damageable->NotifyDanger(Avatar->GetActorLocation(), Avatar);
			}
		}
	}
}

void UCombatAbilityBase::PerformHitCheck(const FGameplayEventData& EventData)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar || !EffectToApply) return;

	// the trace notify tells us which bone the hit came from
	FName DamageSourceBone = NAME_None;
	if (const UAnimNotify_DoAttackTrace* TraceNotify = Cast<UAnimNotify_DoAttackTrace>(EventData.OptionalObject))
	{
		DamageSourceBone = TraceNotify->AttackBoneName;
	}

	const ACharacter* AvatarCharacter = Cast<ACharacter>(Avatar);
	const FVector TraceStart = (AvatarCharacter && DamageSourceBone != NAME_None)
		? AvatarCharacter->GetMesh()->GetSocketLocation(DamageSourceBone)
		: Avatar->GetActorLocation();
	const FVector TraceEnd = TraceStart + (Avatar->GetActorForwardVector() * MeleeTraceDistance);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	if (bDamageWorldObjects)
	{
		ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	}

	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(MeleeTraceRadius);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Avatar);

	TArray<FHitResult> OutHits;
	if (GetWorld()->SweepMultiByObjectType(OutHits, TraceStart, TraceEnd, FQuat::Identity, ObjectParams, CollisionShape, QueryParams))
	{
		for (const FHitResult& CurrentHit : OutHits)
		{
			ApplyDamageToTarget(CurrentHit);
		}
	}
}

void UCombatAbilityBase::ApplyDamageToTarget(const FHitResult& HitResult)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	UAbilitySystemComponent* MyASC = GetAbilitySystemComponentFromActorInfo();
	AActor* TargetActor = HitResult.GetActor();

	if (!Avatar || !MyASC || !TargetActor || !EffectToApply) return;

	UAbilitySystemComponent* TargetASC = TargetActor->FindComponentByClass<UAbilitySystemComponent>();
	if (!TargetASC) return;

	// knock upwards and away from the impact normal
	const FVector Impulse = (HitResult.ImpactNormal * -KnockbackImpulse) + (FVector::UpVector * LaunchImpulse);

	// attacker-side effect context carries the hit result and knockback impulse to the victim
	FGameplayEffectContextHandle Context = MyASC->MakeEffectContext();
	if (FCombatGameplayEffectContext* CombatContext = static_cast<FCombatGameplayEffectContext*>(Context.Get()))
	{
		CombatContext->AddHitResult(HitResult);
		CombatContext->SetKnockbackImpulse(Impulse);
	}

	FGameplayEffectSpecHandle SpecHandle = MyASC->MakeOutgoingSpec(EffectToApply, GetAbilityLevel(), Context);
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(TAG_DataDamage, -AttackDamage);
		MyASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}

	// execute the damage cue on the victim so it can react through its own cue handler
	FGameplayCueParameters CueParams;
	CueParams.EffectContext = Context;
	CueParams.Location = HitResult.ImpactPoint;
	CueParams.Normal = HitResult.ImpactNormal;
	CueParams.Instigator = Avatar;
	CueParams.EffectCauser = Avatar;
	CueParams.RawMagnitude = AttackDamage;
	TargetASC->ExecuteGameplayCue(TAG_GameplayCue_Damage, CueParams);

	// attacker-side feedback
	if (ACharacterBase* Character = Cast<ACharacterBase>(Avatar))
	{
		Character->DealtDamage(AttackDamage, HitResult.ImpactPoint);
	}
}

void UCombatAbilityBase::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCombatAbilityBase::OnAttackTraceEvent(FGameplayEventData EventData)
{
	PerformHitCheck(EventData);
}
