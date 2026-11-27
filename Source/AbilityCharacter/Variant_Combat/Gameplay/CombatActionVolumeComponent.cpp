// Copyright Epic Games, Inc. All Rights Reserved.

#include "CombatActionVolumeComponent.h"
#include "CombatActivatable.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"

UCombatActionVolumeComponent::UCombatActionVolumeComponent()
{
        PrimaryComponentTick.bCanEverTick = false;

        // default prompt text
        UseText = NSLOCTEXT("CombatActionVolume", "DefaultUseText", "Use");
}

void UCombatActionVolumeComponent::OnRegister()
{
        Super::OnRegister();

        if (bCreateTraceBox)
        {
                CreateTraceBox();
        }
}

void UCombatActionVolumeComponent::OnUnregister()
{
        // release the box we created; it is not part of any Blueprint template
        if (IsValid(TraceBox))
        {
                TraceBox->DestroyComponent();
                TraceBox = nullptr;
        }

        Super::OnUnregister();
}

void UCombatActionVolumeComponent::CreateTraceBox()
{
        AActor* Owner = GetOwner();

        // templates and class default objects don't get a box
        if (!Owner || Owner->HasAnyFlags(RF_ClassDefaultObject) || !Owner->GetWorld())
        {
                return;
        }

        // re-registration after a reconstruction keeps the existing box
        if (IsValid(TraceBox))
        {
                return;
        }

        USceneComponent* OwnerRoot = Owner->GetRootComponent();
        if (!OwnerRoot)
        {
                UE_LOG(LogTemp, Warning, TEXT("[ActionVolume] %s has no root component; the trace box for its action volume could not be attached"), *Owner->GetName());
                return;
        }

        TraceBox = NewObject<UBoxComponent>(Owner, TEXT("ActionVolumeTraceBox"));
        check(TraceBox);

        TraceBox->SetBoxExtent(TraceBoxExtent);

        // invisible to movement, but the use ability's interaction ray must find it:
        // the box only responds to the visibility trace channel, so characters walk
        // straight through it while look-at traces hit it
        TraceBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        TraceBox->SetCollisionObjectType(ECC_WorldDynamic);
        TraceBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
        TraceBox->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);
        TraceBox->SetGenerateOverlapEvents(false);

        // the volume itself is invisible in game; the editor still draws its bounds
        TraceBox->SetHiddenInGame(true);

        TraceBox->SetupAttachment(OwnerRoot);
        TraceBox->SetRelativeLocation(TraceBoxOffset);
        TraceBox->RegisterComponent();
}

bool UCombatActionVolumeComponent::CanUse_Implementation(AActor*)
{
        // single-use volumes stop being usable once they have been used
        return !bSingleUse || !bUsed;
}

FText UCombatActionVolumeComponent::GetUseText_Implementation(AActor*)
{
        return UseText;
}

void UCombatActionVolumeComponent::Use_Implementation(AActor* User)
{
        if (bSingleUse)
        {
                bUsed = true;
        }

        UE_LOG(LogTemp, Warning, TEXT("[ActionVolume] %s's action volume was used by %s"), *GetNameSafe(GetOwner()), User ? *User->GetName() : TEXT("nobody"));

        // notify listeners (Blueprints, StateTree, UI...)
        OnUsed.Broadcast(User);

        // activate the linked actors through the existing activation interface
        for (AActor* ActorToActivate : ActorsToActivate)
        {
                if (ICombatActivatable* Activatable = Cast<ICombatActivatable>(ActorToActivate))
                {
                        Activatable->ActivateInteraction(User);
                }
        }
}

UCombatActionVolumeComponent* UCombatActionVolumeComponent::FindActionVolume(const AActor* Actor)
{
        if (!Actor)
        {
                return nullptr;
        }

        TInlineComponentArray<UCombatActionVolumeComponent*> Volumes(Actor);
        return Volumes.Num() > 0 ? Volumes[0] : nullptr;
}
