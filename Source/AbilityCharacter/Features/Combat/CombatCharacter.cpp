// Copyright Epic Games, Inc. All Rights Reserved.


#include "CombatCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "Features/Combat/UI/CombatLifeBar.h"
#include "Engine/DamageEvents.h"
#include "TimerManager.h"
#include "Engine/LocalPlayer.h"
#include "Features/Combat/CombatPlayerController.h"
#include "Core/AbilitySystem/CombatGameplayTags.h"
#include "Features/Combat/AbilitySystem/StanceMonitorAbility.h"
#include "Features/Combat/AbilitySystem/CombatStanceAbility.h"
#include "Features/Combat/AbilitySystem/NeutralStanceAbility.h"

ACombatCharacter::ACombatCharacter()
{
        PrimaryActorTick.bCanEverTick = true;

        // Set size for collision capsule
        GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);

        // Configure character movement
        GetCharacterMovement()->MaxWalkSpeed = 400.0f;

        // create the camera boom
        CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
        CameraBoom->SetupAttachment(RootComponent);

        CameraBoom->TargetArmLength = DefaultCameraDistance;
        CameraBoom->bUsePawnControlRotation = true;
        CameraBoom->bEnableCameraLag = true;
        CameraBoom->bEnableCameraRotationLag = true;

        // create the orbiting camera
        FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
        FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
        FollowCamera->bUsePawnControlRotation = false;

        // set the player tag
        Tags.Add(FName("Player"));

        // the stance monitor watches the game context (nearby enemies, arena volumes)
        // and switches between stances; the combat stance grants the attack abilities.
        // Dash demonstrates the cost effect + gameplay cue chain.
        DefaultAbilities = {
                UStanceMonitorAbility::StaticClass(),
                UCombatStanceAbility::StaticClass(),
                UNeutralStanceAbility::StaticClass()
        };

        // shared attributes with the AI characters
        static ConstructorHelpers::FClassFinder<UGameplayEffect> InitialAttributesFinder(TEXT("/Game/Variant_Combat/Effect/GE_InitialCharacter"));
        if (InitialAttributesFinder.Succeeded())
        {
                DefaultAttributeEffect = InitialAttributesFinder.Class;
        }

        // dash is granted but not bound to input yet; activate it by the Ability.Dash tag
        static ConstructorHelpers::FClassFinder<UGameplayAbility> DashAbilityFinder(TEXT("/Game/Variant_Combat/Blueprints/Abilities/Dash/GA_Dash"));
        if (DashAbilityFinder.Succeeded())
        {
                DefaultAbilities.Add(DashAbilityFinder.Class);
        }
}

void ACombatCharacter::Move(const FInputActionValue& Value)
{
        // input is a Vector2D
        FVector2D MovementVector = Value.Get<FVector2D>();

        // route the input
        DoMove(MovementVector.X, MovementVector.Y);
}

void ACombatCharacter::Look(const FInputActionValue& Value)
{
        FVector2D LookAxisVector = Value.Get<FVector2D>();

        // route the input
        DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ACombatCharacter::ComboAttackPressed()
{
        // route the input
        DoComboAttackStart();
}

void ACombatCharacter::ChargedAttackPressed()
{
        // route the input
        DoChargedAttackStart();
}

void ACombatCharacter::ChargedAttackReleased()
{
        // route the input
        DoChargedAttackEnd();
}

void ACombatCharacter::ToggleCamera()
{
        // call the BP hook
        BP_ToggleCamera();
}

void ACombatCharacter::DoMove(float Right, float Forward)
{
        if (GetController() != nullptr)
        {
                // find out which way is forward
                const FRotator Rotation = GetController()->GetControlRotation();
                const FRotator YawRotation(0, Rotation.Yaw, 0);

                // get forward vector
                const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

                // get right vector
                const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

                // add movement
                AddMovementInput(ForwardDirection, Forward);
                AddMovementInput(RightDirection, Right);
        }
}

void ACombatCharacter::DoLook(float Yaw, float Pitch)
{
        if (GetController() != nullptr)
        {
                // add yaw and pitch input to controller
                AddControllerYawInput(Yaw);
                AddControllerPitchInput(Pitch);
        }
}

void ACombatCharacter::DoComboAttackStart()
{
        // forward the input to the ability system: the first press activates the
        // combo attack ability, further presses buffer the next combo hit
        SendAbilityInputEvent(TAG_Event_InputPrimaryPressed);
}

void ACombatCharacter::DoComboAttackEnd()
{
        // stub
}

void ACombatCharacter::DoChargedAttackStart()
{
        // forward the input to the ability system: the press activates the charged
        // attack ability, which keeps charging while we hold the button
        SendAbilityInputEvent(TAG_Event_InputSecondaryPressed);
}

void ACombatCharacter::DoChargedAttackEnd()
{
        // let the charged attack ability know the input was released
        SendAbilityInputEvent(TAG_Event_InputSecondaryReleased);
}

void ACombatCharacter::HandleDeath()
{
        // disable movement while we're dead
        GetCharacterMovement()->DisableMovement();

        // enable full ragdoll physics
        GetMesh()->SetSimulatePhysics(true);

        // pull back the camera
        GetCameraBoom()->TargetArmLength = DeathCameraDistance;

        // schedule respawning
        GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &ACombatCharacter::RespawnCharacter, RespawnTime, false);
}

void ACombatCharacter::RespawnCharacter()
{
        // destroy the character and let it be respawned by the Player Controller
        Destroy();
}

void ACombatCharacter::BeginPlay()
{
        Super::BeginPlay();

        // initialize the camera
        GetCameraBoom()->TargetArmLength = DefaultCameraDistance;

        // save the relative transform for the mesh so we can reset the ragdoll later
        MeshStartingTransform = GetMesh()->GetRelativeTransform();
}

void ACombatCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
        Super::EndPlay(EndPlayReason);

        // clear the respawn timer
        GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);
}

void ACombatCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
        Super::SetupPlayerInputComponent(PlayerInputComponent);

        // Set up action bindings
        if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
        {
                // Moving
                EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACombatCharacter::Move);

                // Looking
                EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACombatCharacter::Look);
                EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ACombatCharacter::Look);

                // Combo Attack
                EnhancedInputComponent->BindAction(ComboAttackAction, ETriggerEvent::Started, this, &ACombatCharacter::ComboAttackPressed);

                // Charged Attack
                EnhancedInputComponent->BindAction(ChargedAttackAction, ETriggerEvent::Started, this, &ACombatCharacter::ChargedAttackPressed);
                EnhancedInputComponent->BindAction(ChargedAttackAction, ETriggerEvent::Completed, this, &ACombatCharacter::ChargedAttackReleased);

                // Jump Action
                EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACombatCharacter::Jump );

                // Camera Side Toggle
                EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Triggered, this, &ACombatCharacter::ToggleCamera);
        }
}

void ACombatCharacter::NotifyControllerChanged()
{
        Super::NotifyControllerChanged();

        // update the respawn transform on the Player Controller
        if (ACombatPlayerController* PC = Cast<ACombatPlayerController>(GetController()))
        {
                PC->SetRespawnTransform(GetActorTransform());
        }
}
