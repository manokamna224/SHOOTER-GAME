// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShooterGameGameMode.generated.h"

/**
 * Custom GameMode for the open-world shooter.
 * 
 * Responsibilities:
 *   - Sets the default Pawn class and Player Controller
 *   - Manages spawn locations across the open world
 *   - Handles dynamic difficulty and encounter spawning
 *   - Coordinates vehicle <-> infantry possession transitions
 */
UCLASS()
class SHOOTERGAME_API AShooterGameGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AShooterGameGameMode();

protected:
    virtual void BeginPlay() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;

public:
    /** Default pawn class used when the player spawns on foot */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Classes")
    TSubclassOf<APawn> DefaultInfantryClass;

    /** Vehicle class spawned for the player if they choose to start in a vehicle */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Classes")
    TSubclassOf<APawn> DefaultVehicleClass;

    /**
     * Called by ABaseVehicle when the player enters/exits a vehicle.
     * Allows the GameMode to track possession state and adjust spawning rules.
     */
    virtual void HandleVehiclePossessionChange(APlayerController* InController, APawn* NewPawn);

protected:
    /** Whether the player should spawn in a vehicle by default */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Spawning")
    bool bStartInVehicle;

    /** Override to select infantry or vehicle pawn based on configuration */
    virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
};
