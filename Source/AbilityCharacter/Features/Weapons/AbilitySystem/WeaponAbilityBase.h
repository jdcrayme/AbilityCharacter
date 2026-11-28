// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "WeaponAbilityBase.generated.h"

/**
 *  Base class for the weapon abilities that do not attack (reload, aim,
 *  equip) - the fire ability instead derives from UCombatAbilityBase so it
 *  inherits the montage -> trace notify -> effect -> cue attack chain.
 *
 *  Weapon abilities never cast to a character class: everything they need
 *  (weapon, data, equipment) resolves through the avatar's equipment
 *  component, so the same ability runs on any pawn the component is on.
 *
 *  State tags (Reloading, Swapping, Aiming) are applied through the
 *  replicated slot mechanism the stance abilities use: the server applies
 *  them through ApplyGrantedTags so they replicate to every client, while
 *  a predicted client instance adds them as loose tags so local gating and
 *  poses react immediately (the same split USetStanceAbility performs).
 */
UCLASS(Abstract)
class ABILITYCHARACTER_API UWeaponAbilityBase : public UGameplayAbility
{
        GENERATED_BODY()

public:

        UWeaponAbilityBase();

protected:

        /**
         *  Applies state tags for a slot: through the replicated slot on the
         *  server, as loose tags on a predicted client instance. Slots must be
         *  dedicated per state (State.Weapon.Reloading, State.Aiming...) because
         *  applying a slot replaces whatever it granted before.
         */
        void ApplyStateSlot(const FGameplayTag& SlotTag, const FGameplayTagContainer& Tags);

        /** Clears a state slot applied by ApplyStateSlot */
        void ClearStateSlot(const FGameplayTag& SlotTag);

private:

        /** Tags currently applied through ApplyStateSlot, so the client-side clear removes exactly what was added */
        FGameplayTagContainer AppliedSlotTags;

        /** Slot the tags above were applied through */
        FGameplayTag AppliedSlotKey;
};
