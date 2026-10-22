#include "BaseVehicle.h"
#include "ShooterGame.h"
#include "Core/BaseCharacter.h"
#include "Core/ShooterGamePlayerController.h"
#include "ShooterGameGameMode.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "Kismet/GameplayStatics.h"

ABaseVehicle::ABaseVehicle()
{
    // Get the Chaos vehicle movement component that AWheeledVehiclePawn creates
    ChaosMovement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());
    // Note: In C++ constructor, the movement component may not be fully initialized yet.
    // We retrieve it again in BeginPlay.

    // Interaction volume at the driver door
    InteractionVolume = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionVolume"));
    InteractionVolume->SetupAttachment(RootComponent);
    InteractionVolume->SetSphereRadius(200.0f);
    InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Vehicle chase camera
    VehicleCameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("VehicleCameraBoom"));
    VehicleCameraBoom->SetupAttachment(RootComponent);
    VehicleCameraBoom->TargetArmLength = 600.0f;
    VehicleCameraBoom->bUsePawnControlRotation = false;
    VehicleCameraBoom->bInheritPitch = false;
    VehicleCameraBoom->bInheritRoll = false;
    VehicleCameraBoom->bInheritYaw = true;
    VehicleCameraBoom->SocketOffset = FVector(0.0f, 0.0f, 150.0f);

    VehicleCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VehicleCamera"));
    VehicleCamera->SetupAttachment(VehicleCameraBoom, USpringArmComponent::SocketName);
    VehicleCamera->bUsePawnControlRotation = false;

    // Defaults
    InteractRange = 300.0f;
    DriverSeatSocketName = TEXT("DriverSeat");
    ExitSocketName = TEXT("ExitPoint");

    bHasDriver = false;
    DriverCharacter = nullptr;
    DriverCachedCollision = ECollisionEnabled::QueryAndPhysics;

    // Input actions are assigned in Blueprint defaults
    ThrottleAction = nullptr;
    SteeringAction = nullptr;
    BrakeAction = nullptr;
    HandbrakeAction = nullptr;
    LookAction = nullptr;
    CameraSwitchAction = nullptr;
}

void ABaseVehicle::BeginPlay()
{
    Super::BeginPlay();

    // Retrieve the Chaos movement component now that the actor is fully initialized
    ChaosMovement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());
    if (!ChaosMovement)
    {
        UE_LOG(LogShooterGame, Error, TEXT("BaseVehicle: Failed to get ChaosWheeledVehicleMovementComponent."));
    }
}

void ABaseVehicle::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void ABaseVehicle::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!EnhancedInput)
    {
        return;
    }

    if (ThrottleAction)
    {
        EnhancedInput->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Throttle);
    }

    if (SteeringAction)
    {
        EnhancedInput->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Steering);
    }

    if (BrakeAction)
    {
        EnhancedInput->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Brake);
    }

    if (HandbrakeAction)
    {
        EnhancedInput->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &ThisClass::Input_HandbrakeStarted);
        EnhancedInput->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &ThisClass::Input_HandbrakeStopped);
    }

    if (LookAction)
    {
        EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Look);
    }
}

// ═══════════════════════════════════════════════════════════════
//  Vehicle interaction & possession
// ═══════════════════════════════════════════════════════════════

bool ABaseVehicle::CanInteract() const
{
    return !bHasDriver;
}

void ABaseVehicle::EnterVehicle(AShooterGamePlayerController* Controller, ABaseCharacter* Character)
{
    if (!Controller || !Character || bHasDriver)
    {
        return;
    }

    UE_LOG(LogShooterGame, Log, TEXT("Entering vehicle: %s"), *GetName());

    // Cache the character reference
    DriverCharacter = Character;
    bHasDriver = true;

    // Cache and disable the character's collision
    USkeletalMeshComponent* CharMesh = Character->GetMesh();
    if (CharMesh)
    {
        DriverCachedCollision = CharMesh->GetCollisionEnabled();
    }

    // Attach the character to the driver seat socket
    if (CharMesh)
    {
        Character->AttachToComponent(
            GetMesh(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            DriverSeatSocketName
        );

        CharMesh->SetVisibility(false);
        CharMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // Also disable the character's collision capsule
    UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
    if (Capsule)
    {
        Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // Unpossess the character, possess the vehicle
    Controller->UnPossess();

    // Notify the GameMode of the possession change
    if (AShooterGameGameMode* GameMode = GetWorld()->GetAuthGameMode<AShooterGameGameMode>())
    {
        GameMode->HandleVehiclePossessionChange(Controller, this);
    }

    Controller->Possess(this);

    // Notify Blueprint
    Controller->BP_OnEnteredVehicle(this);
}

void ABaseVehicle::ExitVehicle(AShooterGamePlayerController* Controller, ABaseCharacter* Character)
{
    if (!Controller || !DriverCharacter)
    {
        return;
    }

    UE_LOG(LogShooterGame, Log, TEXT("Exiting vehicle: %s"), *GetName());

    // Unpossess the vehicle
    Controller->UnPossess();

    // Detach the character from the vehicle
    ABaseCharacter* Char = DriverCharacter;
    Char->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    // Restore visibility and collision
    USkeletalMeshComponent* CharMesh = Char->GetMesh();
    if (CharMesh)
    {
        CharMesh->SetVisibility(true);
        CharMesh->SetCollisionEnabled(DriverCachedCollision);
    }

    UCapsuleComponent* Capsule = Char->GetCapsuleComponent();
    if (Capsule)
    {
        Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }

    // Find a safe exit location and teleport the character there
    FVector SafeExit = GetSafeExitLocation();
    Char->SetActorLocation(SafeExit, false, nullptr, ETeleportType::ResetPhysics);

    // Notify the GameMode
    if (AShooterGameGameMode* GameMode = GetWorld()->GetAuthGameMode<AShooterGameGameMode>())
    {
        GameMode->HandleVehiclePossessionChange(Controller, Char);
    }

    // Re-possess the character
    Controller->Possess(Char);

    // Clear state
    bHasDriver = false;
    DriverCharacter = nullptr;

    // Notify Blueprint
    Controller->BP_OnExitedVehicle(this);
}

FVector ABaseVehicle::GetSafeExitLocation() const
{
    // Start the raycast from the exit socket position
    FVector Start = GetActorLocation();
    USkeletalMeshComponent* Mesh = GetMesh();
    if (Mesh && Mesh->DoesSocketExist(ExitSocketName))
    {
        Start = Mesh->GetSocketLocation(ExitSocketName);
    }

    FVector End = Start - FVector(0.0f, 0.0f, 500.0f); // Raycast downward

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(DriverCharacter);

    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        // Offset slightly above the ground to avoid clipping
        return Hit.ImpactPoint + FVector(0.0f, 0.0f, 100.0f);
    }

    // Fallback: use the start position offset to the side
    return Start + FVector(150.0f, 0.0f, 0.0f);
}

// ═══════════════════════════════════════════════════════════════
//  Input handlers
// ═══════════════════════════════════════════════════════════════

void ABaseVehicle::Input_Throttle(const FInputActionValue& Value)
{
    if (!ChaosMovement)
    {
        return;
    }

    const float Throttle = Value.Get<float>();
    ChaosMovement->SetThrottleInput(Throttle);
}

void ABaseVehicle::Input_Steering(const FInputActionValue& Value)
{
    if (!ChaosMovement)
    {
        return;
    }

    const float Steering = Value.Get<float>();
    ChaosMovement->SetSteeringInput(Steering);
}

void ABaseVehicle::Input_Brake(const FInputActionValue& Value)
{
    if (!ChaosMovement)
    {
        return;
    }

    const float Brake = Value.Get<float>();
    ChaosMovement->SetBrakeInput(Brake);
}

void ABaseVehicle::Input_HandbrakeStarted(const FInputActionValue& Value)
{
    if (!ChaosMovement)
    {
        return;
    }

    ChaosMovement->SetHandbrakeInput(true);
}

void ABaseVehicle::Input_HandbrakeStopped(const FInputActionValue& Value)
{
    if (!ChaosMovement)
    {
        return;
    }

    ChaosMovement->SetHandbrakeInput(false);
}

void ABaseVehicle::Input_Look(const FInputActionValue& Value)
{
    const FInputActionValue::Axis2D LookAxis = Value.Get<FInputActionValue::Axis2D>();
    AddControllerYawInput(LookAxis.X);
    AddControllerPitchInput(LookAxis.Y);
}
