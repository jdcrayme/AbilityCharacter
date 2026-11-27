// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "CombatEffectContext.generated.h"

/**
 *  Custom gameplay effect context for combat.
 *  Extends the base context with the knockback impulse computed by the attacking
 *  ability, so the victim can translate a GameplayCue.Damage event into a physical
 *  reaction without the attacker touching the victim's components directly.
 */
USTRUCT()
struct ABILITYCHARACTER_API FCombatGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public:

	/** Impulse the victim should receive as part of the damage reaction */
	UPROPERTY()
	FVector KnockbackImpulse = FVector::ZeroVector;

	void SetKnockbackImpulse(const FVector& InImpulse) { KnockbackImpulse = InImpulse; }
	const FVector& GetKnockbackImpulse() const { return KnockbackImpulse; }

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FCombatGameplayEffectContext::StaticStruct();
	}

	virtual FCombatGameplayEffectContext* Duplicate() const override
	{
		FCombatGameplayEffectContext* NewContext = new FCombatGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;
};

template<>
struct TStructOpsTypeTraits<FCombatGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FCombatGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};

/** Casts a context handle to the combat context. Returns nullptr for foreign contexts */
namespace CombatEffectContext
{
	ABILITYCHARACTER_API const FCombatGameplayEffectContext* Extract(const FGameplayEffectContextHandle& Handle);
}
