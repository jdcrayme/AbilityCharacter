// Copyright Epic Games, Inc. All Rights Reserved.

#include "RangedAbility.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

URangedAbility::URangedAbility()
{
	// ranged attacks trace from the weapon muzzle by default
	MuzzleSocketName = TEXT("muzzle");
	MeleeTraceDistance = 0.0f;
}

void URangedAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar) return;

	// trace from the avatar's view if possible, otherwise straight from the avatar
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
	if (GetWorld()->SweepSingleByObjectType(HitResult, TraceStart, TraceEnd, FQuat::Identity, ObjectParams, FCollisionShape::MakeSphere(MeleeTraceRadius), QueryParams))
	{
		ApplyDamageToTarget(HitResult);
	}
}
