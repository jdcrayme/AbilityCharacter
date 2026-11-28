// Copyright Epic Games, Inc. All Rights Reserved.

#include "WeaponAbilityBase.h"

#include "AbilitySystemComponent.h"
#include "Core/AbilitySystem/CombatAbilitySystemComponent.h"

UWeaponAbilityBase::UWeaponAbilityBase()
{
        // one instance per actor: re-pressing while active reaches the running
        // instance (or is rejected by the gates) instead of starting a parallel one
        InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
        bRetriggerInstancedAbility = false;

        // weapon actions are predicted: the montage plays locally for
        // responsiveness while the server owns the state changes
        NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UWeaponAbilityBase::ApplyStateSlot(const FGameplayTag& SlotTag, const FGameplayTagContainer& Tags)
{
        UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
        if (!ASC || !SlotTag.IsValid() || Tags.IsEmpty())
        {
                return;
        }

        AActor* Avatar = GetAvatarActorFromActorInfo();
        const bool bServerInstance = Avatar && Avatar->HasAuthority();

        // remember what this instance applied so the clear removes exactly that
        AppliedSlotKey = SlotTag;
        AppliedSlotTags = Tags;

        if (bServerInstance)
        {
                // the replicated slot: effect-granted tags reach every client through
                // regular gameplay effect replication (see UCombatTagGrantEffect)
                if (UCombatAbilitySystemComponent* CombatASC = Cast<UCombatAbilitySystemComponent>(ASC))
                {
                        CombatASC->ApplyGrantedTags(SlotTag, Tags);
                }
        }
        else
        {
                // predicted client instance: local tags for immediate gating and pose
                // feedback; the server's replicated slot catches up a moment later
                for (const FGameplayTag& Tag : Tags)
                {
                        ASC->AddLooseGameplayTag(Tag);
                }
        }
}

void UWeaponAbilityBase::ClearStateSlot(const FGameplayTag& SlotTag)
{
        UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
        if (!ASC || !SlotTag.IsValid())
        {
                return;
        }

        AActor* Avatar = GetAvatarActorFromActorInfo();
        const bool bServerInstance = Avatar && Avatar->HasAuthority();

        if (bServerInstance)
        {
                if (UCombatAbilitySystemComponent* CombatASC = Cast<UCombatAbilitySystemComponent>(ASC))
                {
                        CombatASC->ClearGrantedTags(SlotTag);
                }
        }
        else if (AppliedSlotKey == SlotTag)
        {
                // remove exactly the loose tags this instance added; the server's
                // replicated slot is cleared by the server instance
                for (const FGameplayTag& Tag : AppliedSlotTags)
                {
                        ASC->RemoveLooseGameplayTag(Tag);
                }
        }

        if (AppliedSlotKey == SlotTag)
        {
                AppliedSlotTags.Reset();
                AppliedSlotKey = FGameplayTag();
        }
}
