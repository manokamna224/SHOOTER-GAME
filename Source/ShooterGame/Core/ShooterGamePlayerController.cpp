#include "ShooterGamePlayerController.h"
#include "ShooterGame.h"
#include "Core/BaseCharacter.h"
#include "Core/BaseVehicle.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "CollisionShape.h"

AShooterGamePlayerController::AShooterGamePlayerController()
{
    InfantryInputMappingContext = nullptr;
    VehicleInputMappingContext = nullptr;
    InteractAction = nullptr;
    ExitVehicleAction = nullptr;
}

void AShooterGamePlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
    if (!EnhancedInput)
    {
        UE_LOG(LogShooterGame, Error, TEXT("SetupInputComponent: InputComponent is not an EnhancedInputComponent."));
        return;
    }

    if (InteractAction)
    {
        EnhancedInput->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ThisClass::Input_InteractTriggered);
    }

    if (ExitVehicleAction)
    {
        EnhancedInput->BindAction(ExitVehicleAction, ETriggerEvent::Triggered, this, &ThisClass::Input_ExitVehicleTriggered);
    }
}

void AShooterGamePlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (ABaseCharacter* Char = Cast<ABaseCharacter>(InPawn))
    {
        CachedCharacter = Char;
        CurrentVehicle = nullptr;
        SwitchInputContext(false);
    }
    else if (ABaseVehicle* Vehicle = Cast<ABaseVehicle>(InPawn))
    {
        CurrentVehicle = Vehicle;
        SwitchInputContext(true);
    }
}

void AShooterGamePlayerController::OnUnPossess()
{
    Super::OnUnPossess();
}

void AShooterGamePlayerController::TryEnterVehicle()
{
    if (CurrentVehicle.IsValid())
    {
        // Already in a vehicle
        return;
    }

    ABaseCharacter* Character = CachedCharacter.Get();
    if (!Character)
    {
        UE_LOG(LogShooterGame, Warning, TEXT("TryEnterVehicle: No cached character."));
        return;
    }

    // Find the nearest interactable vehicle within range
    TArray<AActor*> Vehicles;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseVehicle::StaticClass(), Vehicles);

    ABaseVehicle* NearestVehicle = nullptr;
    float NearestDistSq = MAX_FLT;

    for (AActor* VehicleActor : Vehicles)
    {
        if (ABaseVehicle* Vehicle = Cast<ABaseVehicle>(VehicleActor))
        {
            if (!Vehicle->CanInteract())
            {
                continue;
            }

            float DistSq = FVector::DistSquared2D(Character->GetActorLocation(), Vehicle->GetActorLocation());
            if (DistSq < NearestDistSq && DistSq <= FMath::Square(Vehicle->GetInteractRange()))
            {
                NearestDistSq = DistSq;
                NearestVehicle = Vehicle;
            }
        }
    }

    if (NearestVehicle)
    {
        NearestVehicle->EnterVehicle(this, Character);
    }
    else
    {
        UE_LOG(LogShooterGame, Log, TEXT("TryEnterVehicle: No vehicle in range."));
    }
}

void AShooterGamePlayerController::TryExitVehicle()
{
    if (!CurrentVehicle.IsValid())
    {
        return;
    }

    CurrentVehicle->ExitVehicle(this, CachedCharacter.Get());
}

void AShooterGamePlayerController::SwitchInputContext(bool bToVehicle)
{
    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    if (!LocalPlayer)
    {
        return;
    }

    UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
    if (!Subsystem)
    {
        return;
    }

    // Remove the current context, add the new one
    if (bToVehicle)
    {
        if (InfantryInputMappingContext)
        {
            Subsystem->RemoveMappingContext(InfantryInputMappingContext);
        }
        if (VehicleInputMappingContext)
        {
            Subsystem->AddMappingContext(VehicleInputMappingContext, 0);
        }
    }
    else
    {
        if (VehicleInputMappingContext)
        {
            Subsystem->RemoveMappingContext(VehicleInputMappingContext);
        }
        if (InfantryInputMappingContext)
        {
            Subsystem->AddMappingContext(InfantryInputMappingContext, 0);
        }
    }
}

void AShooterGamePlayerController::Input_InteractTriggered()
{
    // If currently on foot, try to enter a vehicle.
    // If currently in a vehicle, try to exit.
    if (CurrentVehicle.IsValid())
    {
        TryExitVehicle();
    }
    else
    {
        TryEnterVehicle();
    }
}

void AShooterGamePlayerController::Input_ExitVehicleTriggered()
{
    TryExitVehicle();
}
