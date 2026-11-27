// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatLavaFloor.generated.h"

class UStaticMeshComponent;
class UPrimitiveComponent;
class UGameplayEffect;

/**
 *  A basic actor that applies damage on contact. Targets with an ability system
 *  component receive a standard damage GameplayEffect; anything else falls back
 *  to the ICombatDamageable interface.
 */
UCLASS(abstract)
class ABILITYCHARACTER_API ACombatLavaFloor : public AActor
{
	GENERATED_BODY()

	/** Floor mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Mesh;

protected:

	/** Amount of damage to deal on contact */
	UPROPERTY(EditAnywhere, Category="Damage")
	float Damage = 10000.0f;

	/** Gameplay effect applied to targets with an ability system component */
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

public:

	/** Constructor */
	ACombatLavaFloor();

protected:

	/** Blocking hit handler */
	UFUNCTION()
	void OnFloorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
