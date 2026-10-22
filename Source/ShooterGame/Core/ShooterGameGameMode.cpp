#include "ShooterGameGameMode.h"
#include "ShooterGame.h"
#include "Core/BaseCharacter.h"
#include "Core/BaseVehicle.h"
#include "Kismet/GameplayStatics.h"

AShooterGameGameMode::AShooterGameGameMode()
{
    // Default classes - set in Blueprints or a data table for production
    DefaultPawnClass = ABaseCharacter::StaticClass();
    PlayerControllerClass = nullptr; // Set in DefaultEngine.ini or Blueprint

    DefaultInfantryClass = ABaseCharacter::StaticClass();
    DefaultVehicleClass = nullptr;
    bStartInVehicle = false;
}

void AShooterGameGameMode::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogShooterGame, Log, TEXT("ShooterGameGameMode started."));
}

void AShooterGameGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (NewPlayer)
    {
        UE_LOG(LogShooterGame, Log, TEXT("Player joined: %s"), *NewPlayer->GetName());
    }
}

void AShooterGameGameMode::HandleVehiclePossessionChange(APlayerController* InController, APawn* NewPawn)
{
    if (!InController || !NewPawn)
    {
        return;
    }

    UE_LOG(LogShooterGame, Log, TEXT("Possession changed for %s -> %s"),
        *InController->GetName(),
        *NewPawn->GetName());
}

UClass* AShooterGameGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
    if (bStartInVehicle && DefaultVehicleClass)
    {
        return DefaultVehicleClass;
    }

    if (DefaultInfantryClass)
    {
        return DefaultInfantryClass;
    }

    return Super::GetDefaultPawnClassForController_Implementation(InController);
}
