// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Core/Characters/CharacterBase.h"
#include "CombatCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;
class UCombatLifeBar;
class UWidgetComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogCombatCharacter, Log, All);

/**
 *  An enhanced Third Person Character with melee combat capabilities.
 *  All attacks are GameplayAbilities; input is forwarded to the ability system
 *  as gameplay events, and damage reactions run through the GameplayCue.Damage cue.
 *  - Combo attack string
 *  - Press and hold charged attack
 *  - Damage dealing and reaction
 *  - Death
 *  - Respawning
 */
UCLASS(abstract)
class ABILITYCHARACTER_API ACombatCharacter : public ACharacterBase
{
        GENERATED_BODY()

        /** Camera boom positioning the camera behind the character */
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
        USpringArmComponent* CameraBoom;

        /** Follow camera */
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
        UCameraComponent* FollowCamera;

protected:

        /** Jump Input Action */
        UPROPERTY(EditAnywhere, Category ="Input")
        UInputAction* JumpAction;

        /** Move Input Action */
        UPROPERTY(EditAnywhere, Category ="Input")
        UInputAction* MoveAction;

        /** Look Input Action */
        UPROPERTY(EditAnywhere, Category ="Input")
        UInputAction* LookAction;

        /** Mouse Look Input Action */
        UPROPERTY(EditAnywhere, Category="Input")
        UInputAction* MouseLookAction;

        /** Combo Attack Input Action */
        UPROPERTY(EditAnywhere, Category ="Input")
        UInputAction* ComboAttackAction;

        /** Charged Attack Input Action */
        UPROPERTY(EditAnywhere, Category ="Input")
        UInputAction* ChargedAttackAction;

        /** Toggle Camera Side Input Action */
        UPROPERTY(EditAnywhere, Category ="Input")
        UInputAction* ToggleCameraAction;

        /** Life bar widget fill color */
        UPROPERTY(EditAnywhere, Category="Damage")
        FLinearColor LifeBarColor;

        /** Camera boom length while the character is dead */
        UPROPERTY(EditAnywhere, Category="Camera", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm"))
        float DeathCameraDistance = 400.0f;

        /** Camera boom length when the character respawns */
        UPROPERTY(EditAnywhere, Category="Camera", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm"))
        float DefaultCameraDistance = 100.0f;

        /** Time to wait before respawning the character */
        UPROPERTY(EditAnywhere, Category="Respawn", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
        float RespawnTime = 3.0f;

        /** Character respawn timer */
        FTimerHandle RespawnTimer;

        /** Copy of the mesh's transform so we can reset it after ragdoll animations */
        FTransform MeshStartingTransform;

public:

        /** Constructor */
        ACombatCharacter();

protected:

        /** Called for movement input */
        void Move(const FInputActionValue& Value);

        /** Called for looking input */
        void Look(const FInputActionValue& Value);

        /** Called for combo attack input */
        void ComboAttackPressed();

        /** Called for combo attack input pressed */
        void ChargedAttackPressed();

        /** Called for combo attack input released */
        void ChargedAttackReleased();

        /** Called for toggle camera side input */
        void ToggleCamera();

        /** BP hook to animate the camera side switch */
        UFUNCTION(BlueprintImplementableEvent, Category="Combat")
        void BP_ToggleCamera();

public:

        /** Handles move inputs from either controls or UI interfaces */
        UFUNCTION(BlueprintCallable, Category="Input")
        virtual void DoMove(float Right, float Forward);

        /** Handles look inputs from either controls or UI interfaces */
        UFUNCTION(BlueprintCallable, Category="Input")
        virtual void DoLook(float Yaw, float Pitch);

        /** Handles combo attack pressed from either controls or UI interfaces */
        UFUNCTION(BlueprintCallable, Category="Input")
        virtual void DoComboAttackStart();

        /** Handles combo attack released from either controls or UI interfaces */
        UFUNCTION(BlueprintCallable, Category="Input")
        virtual void DoComboAttackEnd();

        /** Handles charged attack pressed from either controls or UI interfaces */
        UFUNCTION(BlueprintCallable, Category="Input")
        virtual void DoChargedAttackStart();

        /** Handles charged attack released from either controls or UI interfaces */
        UFUNCTION(BlueprintCallable, Category="Input")
        virtual void DoChargedAttackEnd();

protected:

        /** Called from the respawn timer to destroy and re-create the character */
        void RespawnCharacter();

public:

        /** Initialization */
        virtual void BeginPlay() override;

        /** Cleanup */
        virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

        /** Handles input bindings */
        virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

        /** Handles possessed initialization */
        virtual void NotifyControllerChanged() override;

        /** Handles death events */
        virtual void HandleDeath() override;

public:

        /** Returns CameraBoom subobject **/
        FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

        /** Returns FollowCamera subobject **/
        FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
