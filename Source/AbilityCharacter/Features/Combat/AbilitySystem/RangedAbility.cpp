// Copyright Epic Games, Inc. All Rights Reserved.

#include "RangedAbility.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

URangedAbility::URangedAbility()
{
	// ranged attacks trace from the weapon muzzle by default
	MuzzleSocketName = TEXT("muzzle");

	// players want their shots predicted: the montage plays locally, the server
	// owns the damage (CombatAbilityBase gates the damage paths on authority)
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void URangedAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// one shot per activation; the trace notify (or the montage ending) sets it
	bHasFired = false;

	// commit, play the montage and listen for the trace notify
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void URangedAbility::OnAttackTraceEvent(FGameplayEventData EventData)
{
	// the notify is the fire frame; ignore any additional notifies in the montage
	if (bHasFired)
	{
		return;
	}

	bHasFired = true;
	PerformRangedTrace();
}

void URangedAbility::OnMontageFinished()
{
	// montages without a trace notify still fire once, at the end instead of the
	// exact fire frame
	if (!bHasFired && MontageToPlay)
	{
		bHasFired = true;
		PerformRangedTrace();
	}

	Super::OnMontageFinished();
}

void URangedAbility::PerformRangedTrace()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar) return;

	// the server's instance applies the damage; predicted client instances only
	// played the montage for responsiveness
	if (!Avatar->HasAuthority()) return;

	// trace from the avatar's view if possible, otherwise from the muzzle socket
	FVector TraceStart = Avatar->GetActorLocation();
	FVector TraceDirection = Avatar->GetActorForwardVector();

	if (const APlayerController* PC = Cast<APlayerController>(Avatar->GetInstigatorController()))
	{
		FRotator ViewRotation;
		PC->GetPlayerViewPoint(TraceStart, ViewRotation);
		TraceDirection = ViewRotation.Vector();
	}
	else if (const ACharacter* AvatarCharacter = Cast<ACharacter>(Avatar))
	{
		const FVector MuzzleLocation = AvatarCharacter->GetMesh()->GetSocketLocation(MuzzleSocketName);
		if (!MuzzleLocation.IsNearlyZero())
		{
			TraceStart = MuzzleLocation;
		}
	}

	const FVector TraceEnd = TraceStart + (TraceDirection * TraceDistance);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	if (bDamageWorldObjects)
	{
		ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	}

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Avatar);

	FHitResult HitResult;
	if (GetWorld()->SweepSingleByObjectType(HitResult, TraceStart, TraceEnd, FQuat::Identity, ObjectParams, FCollisionShape::MakeSphere(TraceRadius), QueryParams))
	{
		ApplyDamageToTarget(HitResult);
	}
}
