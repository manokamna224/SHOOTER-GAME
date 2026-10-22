// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputTriggers.h"
#include "ShooterGamePlayerController.generated.h"

// Forward declarations
class UInputMappingContext;
class UInputAction;
class ABaseVehicle;
class ABaseCharacter;

/**
 * Custom PlayerController bridging infantry and vehicle gameplay.
 *
 * Key features:
 *   - Manages two Input Mapping Contexts (IMC_Infantry, IMC_Vehicle)
 *   - Handles the IA_Interact key press for vehicle enter/exit
 *   - Caches the current pawn type to route input correctly
 *   - Provides Blueprint-callable hooks for HUD transitions
 */
UCLASS()
class SHOOTERGAME_API AShooterGamePlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AShooterGamePlayerController();

    // ─────────────────────────────────────────────────────────────
    //  APlayerController interface
    // ─────────────────────────────────────────────────────────────
    virtual void SetupInputComponent() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // ─────────────────────────────────────────────────────────────
    //  Vehicle interaction
    // ─────────────────────────────────────────────────────────────

    /** Attempt to enter the nearest vehicle within interaction range */
    UFUNCTION(BlueprintCallable, Category = "ShooterGame|Vehicle")
    void TryEnterVehicle();

    /** Exit the currently possessed vehicle and respawn the infantry character */
    UFUNCTION(BlueprintCallable, Category = "ShooterGame|Vehicle")
    void TryExitVehicle();

protected:
    // ─────────────────────────────────────────────────────────────
    //  Input Mapping Contexts
    // ─────────────────────────────────────────────────────────────

    /** IMC used while on foot (movement, look, sprint, crouch, aim, fire, reload) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Input")
    TObjectPtr<UInputMappingContext> InfantryInputMappingContext;

    /** IMC used while driving a vehicle (steering, throttle, brake, handbrake, look) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Input")
    TObjectPtr<UInputMappingContext> VehicleInputMappingContext;

    /** Input action for the interact key (enter/exit vehicle) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Input")
    TObjectPtr<UInputAction> InteractAction;

    /** Input action for exiting the vehicle (can be same as InteractAction) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Input")
    TObjectPtr<UInputAction> ExitVehicleAction;

    // ─────────────────────────────────────────────────────────────
    //  Cached references
    // ─────────────────────────────────────────────────────────────

    /** Cached pointer to the vehicle the player is currently driving (null when on foot) */
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "ShooterGame|State")
    TWeakObjectPtr<ABaseVehicle> CurrentVehicle;

    /** Cached pointer to the infantry character (hidden while driving) */
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "ShooterGame|State")
    TWeakObjectPtr<ABaseCharacter> CachedCharacter;

    // ─────────────────────────────────────────────────────────────
    //  Internal helpers
    // ─────────────────────────────────────────────────────────────

    /** Switches the active Input Mapping Context between infantry and vehicle */
    void SwitchInputContext(bool bToVehicle);

    /** Blueprint hook called after entering a vehicle (for HUD/camera swaps) */
    UFUNCTION(BlueprintImplementableEvent, Category = "ShooterGame|Vehicle", meta = (DisplayName = "On Entered Vehicle"))
    void BP_OnEnteredVehicle(ABaseVehicle* EnteredVehicle);

    /** Blueprint hook called after exiting a vehicle */
    UFUNCTION(BlueprintImplementableEvent, Category = "ShooterGame|Vehicle", meta = (DisplayName = "On Exited Vehicle"))
    void BP_OnExitedVehicle(ABaseVehicle* ExitedVehicle);

private:
    // ─────────────────────────────────────────────────────────────
    //  Input handlers
    // ─────────────────────────────────────────────────────────────

    void Input_InteractTriggered();
    void Input_ExitVehicleTriggered();
};
