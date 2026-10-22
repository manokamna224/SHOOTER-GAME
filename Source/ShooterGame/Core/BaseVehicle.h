// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "BaseVehicle.generated.h"

// Forward declarations
class UChaosWheeledVehicleMovementComponent;
class USphereComponent;
class ABaseCharacter;
class AShooterGamePlayerController;
class USpringArmComponent;
class UCameraComponent;
class UInputAction;

/**
 * Base abstract vehicle for the open-world shooter, built on Chaos Vehicles.
 *
 * Features:
 *   - UWheeledVehiclePawn with ChaosWheeledVehicleMovementComponent
 *   - Interaction sphere volume at the driver door
 *   - Clean possession switch: character hidden + collision disabled while driving
 *   - Input Mapping Context swap handled by the PlayerController
 *   - Exit point socket for safe dismount
 *
 * Blueprint subclasses (e.g. BP_SedanCar) configure:
 *   - Skeletal mesh with wheel bone hierarchy (Root -> Wheel_FL/FR/RL/RR)
 *   - Movement component torque curves, gear ratios, suspension, tire friction
 *   - Camera positions (internal / chase / hood)
 */
UCLASS(Abstract)
class SHOOTERGAME_API ABaseVehicle : public AWheeledVehiclePawn
{
    GENERATED_BODY()

public:
    ABaseVehicle();

    // ─────────────────────────────────────────────────────────────
    //  AActor overrides
    // ─────────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // ─────────────────────────────────────────────────────────────
    //  Vehicle interaction & possession
    // ─────────────────────────────────────────────────────────────

    /** Returns true if a player can enter this vehicle */
    UFUNCTION(BlueprintPure, Category = "ShooterGame|Vehicle")
    virtual bool CanInteract() const;

    /** Returns the interaction radius for entering this vehicle (cm) */
    UFUNCTION(BlueprintPure, Category = "ShooterGame|Vehicle")
    float GetInteractRange() const { return InteractRange; }

    /** Called by the PlayerController to enter this vehicle */
    UFUNCTION(BlueprintCallable, Category = "ShooterGame|Vehicle")
    virtual void EnterVehicle(AShooterGamePlayerController* Controller, ABaseCharacter* Character);

    /** Called by the PlayerController to exit this vehicle */
    UFUNCTION(BlueprintCallable, Category = "ShooterGame|Vehicle")
    virtual void ExitVehicle(AShooterGamePlayerController* Controller, ABaseCharacter* Character);

    /** Returns the cached character (hidden inside the vehicle while driving) */
    UFUNCTION(BlueprintPure, Category = "ShooterGame|Vehicle")
    ABaseCharacter* GetDriverCharacter() const { return DriverCharacter; }

protected:
    // ─────────────────────────────────────────────────────────────
    //  Chaos Vehicle movement
    // ─────────────────────────────────────────────────────────────

    /** Chaos wheeled vehicle movement component (auto-created by AWheeledVehiclePawn) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ShooterGame|Vehicle|Movement")
    TObjectPtr<UChaosWheeledVehicleMovementComponent> ChaosMovement;

    // ─────────────────────────────────────────────────────────────
    //  Interaction
    // ─────────────────────────────────────────────────────────────

    /** Sphere collision volume placed at the driver door for interaction detection */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ShooterGame|Vehicle|Interaction")
    TObjectPtr<USphereComponent> InteractionVolume;

    /** Maximum distance (cm) a character can be from the vehicle to enter it */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Vehicle|Interaction")
    float InteractRange;

    /** Socket name on the vehicle mesh where the character is attached while driving */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Vehicle|Interaction")
    FName DriverSeatSocketName;

    /** Socket name used as the exit spawn offset point */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Vehicle|Interaction")
    FName ExitSocketName;

    // ─────────────────────────────────────────────────────────────
    //  Camera (chase cam while driving)
    // ─────────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ShooterGame|Camera")
    TObjectPtr<USpringArmComponent> VehicleCameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ShooterGame|Camera")
    TObjectPtr<UCameraComponent> VehicleCamera;

    // ─────────────────────────────────────────────────────────────
    //  Enhanced Input (vehicle)
    // ─────────────────────────────────────────────────────────────

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Input")
    TObjectPtr<UInputAction> ThrottleAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Input")
    TObjectPtr<UInputAction> SteeringAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Input")
    TObjectPtr<UInputAction> BrakeAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Input")
    TObjectPtr<UInputAction> HandbrakeAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Input")
    TObjectPtr<UInputAction> LookAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Input")
    TObjectPtr<UInputAction> CameraSwitchAction;

    // ─────────────────────────────────────────────────────────────
    //  State
    // ─────────────────────────────────────────────────────────────

    /** The character currently driving this vehicle (null when empty) */
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "ShooterGame|State")
    TObjectPtr<ABaseCharacter> DriverCharacter;

    /** Whether this vehicle currently has a driver */
    bool bHasDriver;

    /** Saved collision state of the driver character (restored on exit) */
    ECollisionEnabled DriverCachedCollision;

    // ─────────────────────────────────────────────────────────────
    //  Input handlers
    // ─────────────────────────────────────────────────────────────

    void Input_Throttle(const struct FInputActionValue& Value);
    void Input_Steering(const FInputActionValue& Value);
    void Input_Brake(const FInputActionValue& Value);
    void Input_HandbrakeStarted(const FInputActionValue& Value);
    void Input_HandbrakeStopped(const FInputActionValue& Value);
    void Input_Look(const FInputActionValue& Value);

    // ─────────────────────────────────────────────────────────────
    //  Helpers
    // ─────────────────────────────────────────────────────────────

    /** Raycast downward from the exit socket to find a safe ground spawn point */
    FVector GetSafeExitLocation() const;
};
