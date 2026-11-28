// Copyright Epic Games, Inc. All Rights Reserved.

#include "WeaponFireAbility.h"
#include "AbilityCharacter.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Core/AbilitySystem/CombatEffectContext.h"
#include "Core/AbilitySystem/CombatGameplayTags.h"
#include "Core/Characters/CharacterBase.h"
#include "Features/Weapons/Equipment/WeaponEquipmentComponent.h"
#include "Features/Weapons/Weapon.h"
#include "Features/Weapons/WeaponData.h"
#include "Features/Weapons/WeaponStateComponent.h"

UWeaponFireAbility::UWeaponFireAbility()
{
        // generic weapon fire identity: input bindings, Blueprints and AI activate
        // through Ability.Weapon.Fire
        SetAssetTags(FGameplayTagContainer(TAG_Ability_Weapon_Fire));

        // the ability activates from the fire input event, like the charged attack
        // activates from the secondary attack press
        FAbilityTriggerData TriggerData;
        TriggerData.TriggerTag = TAG_Event_InputFirePressed;
        TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
        AbilityTriggers.Add(TriggerData);

        // only a drawn weapon fires, and neither reloads nor swaps interrupt it
        ActivationRequiredTags.AddTag(TAG_State_Weapon_Drawn);
        ActivationBlockedTags.AddTag(TAG_State_Weapon_Reloading);
        ActivationBlockedTags.AddTag(TAG_State_Weapon_Swapping);

        // ammo is the cost, not stamina
        StaminaCost = 0.0f;

        // players want their shots predicted: the montage plays locally while the
        // server owns the magazine and the damage (the authority gates below)
        NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool UWeaponFireAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
        if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
        {
                return false;
        }

        // no weapon (or no magazine) equipped: not fireable
        const AActor* Avatar = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
        const AWeapon* Weapon = UWeaponEquipmentComponent::GetEquippedWeaponOnActor(Avatar);
        if (!Weapon || !Weapon->StateComponent)
        {
                return false;
        }

        // an empty magazine can't fire; the click and the reload prompt are the
        // UI's business, fed by the weapon state component's change delegate
        return !Weapon->StateComponent->IsMagEmpty();
}

void UWeaponFireAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
        // one shot per activation
        bHasFired = false;

        // the montage comes from the weapon, not from ability defaults - this is
        // what lets every weapon own its fire animation without subclasses
        if (const UWeaponData* WeaponData = UWeaponEquipmentComponent::GetEquippedWeaponDataOnActor(GetAvatarActorFromActorInfo()))
        {
                MontageToPlay = WeaponData->FireMontage;
        }

        if (!MontageToPlay)
        {
                // montageless weapons (a turret, a test cube) fire immediately
                bHasFired = true;
                PerformWeaponTrace();
                EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
                return;
        }

        // commit, play the weapon's montage and listen for the trace notify
        Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UWeaponFireAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
        // ammo is this ability's cost and it is consumed in PerformWeaponTrace;
        // there is no stamina cost to apply
}

void UWeaponFireAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
        // the fire interval comes from the weapon: the cooldown effect belongs to
        // the weapon data, and the interval travels through the Data.Cooldown
        // SetByCaller magnitude so one GE asset paces every weapon
        const UWeaponData* WeaponData = UWeaponEquipmentComponent::GetEquippedWeaponDataOnActor(GetAvatarActorFromActorInfo());
        if (!WeaponData || !WeaponData->Firing.CooldownEffect)
        {
                // no cooldown effect means the montage's own length paces fire
                return;
        }

        UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
        if (!ASC) return;

        const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(WeaponData->Firing.CooldownEffect, GetAbilityLevel(), ASC->MakeEffectContext());
        if (SpecHandle.IsValid())
        {
                SpecHandle.Data->SetSetByCallerMagnitude(TAG_Data_Cooldown, WeaponData->Firing.FireInterval);
                ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
        }
}

void UWeaponFireAbility::OnAttackTraceEvent(FGameplayEventData EventData)
{
        // the notify is the fire frame; ignore any additional notifies
        if (bHasFired)
        {
                return;
        }

        bHasFired = true;

        // cosmetic feedback (muzzle flash, sound, shake) runs wherever the fire
        // frame lands: predicted client instances included, server instances too
        if (AWeapon* Weapon = UWeaponEquipmentComponent::GetEquippedWeaponOnActor(GetAvatarActorFromActorInfo()))
        {
                OnWeaponFired(Weapon->GetMuzzleTransform());
        }

        PerformWeaponTrace();
}

void UWeaponFireAbility::OnMontageFinished()
{
        // montages without a trace notify still fire once, at the end
        if (!bHasFired)
        {
                bHasFired = true;
                PerformWeaponTrace();
        }

        Super::OnMontageFinished();
}

void UWeaponFireAbility::PerformWeaponTrace()
{
        AActor* Avatar = GetAvatarActorFromActorInfo();
        AWeapon* Weapon = UWeaponEquipmentComponent::GetEquippedWeaponOnActor(Avatar);
        UWeaponData* WeaponData = UWeaponEquipmentComponent::GetEquippedWeaponDataOnActor(Avatar);
        if (!Avatar || !Weapon || !WeaponData || !Weapon->StateComponent)
        {
                return;
        }

        // the magazine and the damage are the server's: predicted client instances
        // played the montage for responsiveness and leave the state alone
        if (!Avatar->HasAuthority())
        {
                return;
        }

        // pay the cost: one round per shot
        if (!Weapon->StateComponent->ConsumeAmmo(1))
        {
                OnMagEmpty();
                return;
        }

        const FWeaponFiringData& Firing = WeaponData->Firing;

        // trace from the controlling player's view when there is one (so the shot
        // goes where the crosshair points), and from the weapon's muzzle otherwise
        FVector TraceStart;
        FVector TraceDirection;

        if (const APlayerController* PC = Cast<APlayerController>(Avatar->GetInstigatorController()))
        {
                FRotator ViewRotation;
                PC->GetPlayerViewPoint(TraceStart, ViewRotation);
                TraceDirection = ViewRotation.Vector();
        }
        else
        {
                const FTransform MuzzleTransform = Weapon->GetMuzzleTransform();
                TraceStart = MuzzleTransform.GetLocation();
                TraceDirection = MuzzleTransform.GetRotation().Vector();
        }

        const FVector TraceEnd = TraceStart + (TraceDirection * Firing.TraceDistance);

        FCollisionObjectQueryParams ObjectParams;
        ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
        if (Firing.bDamageWorldObjects)
        {
                ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
        }

        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(Avatar);
        QueryParams.AddIgnoredActor(Weapon);

        FHitResult HitResult;
        if (GetWorld()->SweepSingleByObjectType(HitResult, TraceStart, TraceEnd, FQuat::Identity, ObjectParams, FCollisionShape::MakeSphere(Firing.TraceRadius), QueryParams))
        {
                ApplyWeaponDamage(HitResult, Firing);
        }
}

void UWeaponFireAbility::ApplyWeaponDamage(const FHitResult& HitResult, const FWeaponFiringData& Firing)
{
        AActor* Avatar = GetAvatarActorFromActorInfo();
        UAbilitySystemComponent* MyASC = GetAbilitySystemComponentFromActorInfo();
        AActor* TargetActor = HitResult.GetActor();

        // the weapon's damage effect wins; the ability's default covers weapons
        // that don't carry one
        const TSubclassOf<UGameplayEffect> EffectClass = Firing.DamageEffect ? Firing.DamageEffect : EffectToApply;

        if (!Avatar || !MyASC || !TargetActor || !EffectClass) return;

        UAbilitySystemComponent* TargetASC = TargetActor->FindComponentByClass<UAbilitySystemComponent>();
        if (!TargetASC) return;

        // knock upwards and away from the impact normal, with the weapon's numbers
        const FVector Impulse = (HitResult.ImpactNormal * -Firing.KnockbackImpulse) + (FVector::UpVector * Firing.LaunchImpulse);

        // attacker-side effect context carries the hit result and knockback impulse to the victim
        FGameplayEffectContextHandle Context = MyASC->MakeEffectContext();
        if (FCombatGameplayEffectContext* CombatContext = static_cast<FCombatGameplayEffectContext*>(Context.Get()))
        {
                CombatContext->AddHitResult(HitResult);
                CombatContext->SetKnockbackImpulse(Impulse);
        }

        FGameplayEffectSpecHandle SpecHandle = MyASC->MakeOutgoingSpec(EffectClass, GetAbilityLevel(), Context);
        if (SpecHandle.IsValid())
        {
                SpecHandle.Data->SetSetByCallerMagnitude(TAG_DataDamage, -Firing.BaseDamage);
                MyASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
        }

        // execute the damage cue on the victim so it can react through its own cue handler
        FGameplayCueParameters CueParams;
        CueParams.EffectContext = Context;
        CueParams.Location = HitResult.ImpactPoint;
        CueParams.Normal = HitResult.ImpactNormal;
        CueParams.Instigator = Avatar;
        CueParams.EffectCauser = Avatar;
        CueParams.RawMagnitude = Firing.BaseDamage;
        TargetASC->ExecuteGameplayCue(TAG_GameplayCue_Damage, CueParams);

        // attacker-side feedback
        if (ACharacterBase* Character = Cast<ACharacterBase>(Avatar))
        {
                Character->DealtDamage(Firing.BaseDamage, HitResult.ImpactPoint);
        }
}
