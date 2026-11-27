// Copyright Epic Games, Inc. All Rights Reserved.


#include "CombatLavaFloor.h"
#include "Core/Interfaces/CombatDamageable.h"
#include "Core/AbilitySystem/CombatGameplayTags.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "UObject/ConstructorHelpers.h"

ACombatLavaFloor::ACombatLavaFloor()
{
	PrimaryActorTick.bCanEverTick = false;

	// create the mesh
	RootComponent = Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	// bind the hit handler
	Mesh->OnComponentHit.AddDynamic(this, &ACombatLavaFloor::OnFloorHit);

	// effect applied to targets with an ability system component
	static ConstructorHelpers::FClassFinder<UGameplayEffect> DamageEffectFinder(TEXT("/Game/Variant_Combat/Effect/GE_DamageHealth"));
	if (DamageEffectFinder.Succeeded())
	{
		DamageEffectClass = DamageEffectFinder.Class;
	}
}

void ACombatLavaFloor::OnFloorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor) return;

	// prefer the ability system path: apply a standard damage effect to the target
	if (UAbilitySystemComponent* TargetASC = OtherActor->FindComponentByClass<UAbilitySystemComponent>())
	{
		if (DamageEffectClass)
		{
			FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
			EffectContext.AddSourceObject(this);

			FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(DamageEffectClass, 1.f, EffectContext);
			if (SpecHandle.IsValid())
			{
				SpecHandle.Data->SetSetByCallerMagnitude(TAG_DataDamage, -Damage);
				TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);

				// run the damage cue so the target reacts
				FGameplayCueParameters CueParams;
				CueParams.EffectContext = EffectContext;
				CueParams.RawMagnitude = Damage;
				CueParams.Instigator = this;
				CueParams.Location = Hit.ImpactPoint;
				TargetASC->ExecuteGameplayCue(TAG_GameplayCue_Damage, CueParams);
			}

			return;
		}
	}

	// fall back to the damageable interface
	if (ICombatDamageable* Damageable = Cast<ICombatDamageable>(OtherActor))
	{
		Damageable->ApplyDamage(Damage, this, Hit.ImpactPoint, FVector::ZeroVector);
	}
}
