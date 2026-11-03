#include "BaseVehicle.h"
#include "ShooterGame.h"
#include "BaseCharacter.h"
#include "ShooterGamePlayerController.h"
#include "ShooterGameGameMode.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "Kismet/GameplayStatics.h"

ABaseVehicle::ABaseVehicle()
{
    ChaosMovement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());

    InteractionVolume = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionVolume"));
    InteractionVolume->SetupAttachment(RootComponent);
    InteractionVolume->SetSphereRadius(200.0f);
    InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

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

    InteractRange = 300.0f;
    DriverSeatSocketName = TEXT("DriverSeat");
    ExitSocketName = TEXT("ExitPoint");

    bHasDriver = false;
    DriverCharacter = nullptr;
    DriverCachedCollision = ECollisionEnabled::QueryAndPhysics;

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
    if (!EnhancedInput) return;

    if (ThrottleAction)  EnhancedInput->BindAction(ThrottleAction,  ETriggerEvent::Triggered,  this, &ThisClass::Input_Throttle);
    if (SteeringAction)  EnhancedInput->BindAction(SteeringAction,  ETriggerEvent::Triggered,  this, &ThisClass::Input_Steering);
    if (BrakeAction)     EnhancedInput->BindAction(BrakeAction,     ETriggerEvent::Triggered,  this, &ThisClass::Input_Brake);
    if (HandbrakeAction)
    {
        EnhancedInput->BindAction(HandbrakeAction, ETriggerEvent::Started,    this, &ThisClass::Input_HandbrakeStarted);
        EnhancedInput->BindAction(HandbrakeAction, ETriggerEvent::Completed,  this, &ThisClass::Input_HandbrakeStopped);
    }
    if (LookAction) EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Look);
}

bool ABaseVehicle::CanInteract() const { return !bHasDriver; }

void ABaseVehicle::EnterVehicle(AShooterGamePlayerController* InController, ABaseCharacter* InCharacter)
{
    if (!InController || !InCharacter || bHasDriver) return;

    UE_LOG(LogShooterGame, Log, TEXT("Entering vehicle: %s"), *GetName());

    DriverCharacter = InCharacter;
    bHasDriver = true;

    USkeletalMeshComponent* CharMesh = InCharacter->GetMesh();
    if (CharMesh)
    {
        DriverCachedCollision = CharMesh->GetCollisionEnabled();
        InCharacter->AttachToComponent(GetMesh(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            DriverSeatSocketName);
        CharMesh->SetVisibility(false);
        CharMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    UCapsuleComponent* CapsuleComp = InCharacter->GetCapsuleComponent();
    if (CapsuleComp) CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    InController->UnPossess();

    if (AShooterGameGameMode* GM = GetWorld()->GetAuthGameMode<AShooterGameGameMode>())
        GM->HandleVehiclePossessionChange(InController, this);

    InController->Possess(this);

    // Blueprint notification — call via Execute_ pattern since it's BlueprintImplementableEvent
    InController->BP_OnEnteredVehicle(this);
}

void ABaseVehicle::ExitVehicle(AShooterGamePlayerController* InController, ABaseCharacter* InCharacter)
{
    if (!InController || !DriverCharacter) return;

    UE_LOG(LogShooterGame, Log, TEXT("Exiting vehicle: %s"), *GetName());

    InController->UnPossess();

    ABaseCharacter* Char = DriverCharacter;
    Char->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    USkeletalMeshComponent* CharMesh = Char->GetMesh();
    if (CharMesh)
    {
        CharMesh->SetVisibility(true);
        CharMesh->SetCollisionEnabled(DriverCachedCollision);
    }

    UCapsuleComponent* CapsuleComp = Char->GetCapsuleComponent();
    if (CapsuleComp) CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    FVector SafeExit = GetSafeExitLocation();
    Char->SetActorLocation(SafeExit, false, nullptr, ETeleportType::ResetPhysics);

    if (AShooterGameGameMode* GM = GetWorld()->GetAuthGameMode<AShooterGameGameMode>())
        GM->HandleVehiclePossessionChange(InController, Char);

    InController->Possess(Char);

    bHasDriver = false;
    DriverCharacter = nullptr;

    InController->BP_OnExitedVehicle(this);
}

FVector ABaseVehicle::GetSafeExitLocation() const
{
    FVector Start = GetActorLocation();
    // Use VehicleMesh to avoid shadowing AWheeledVehiclePawn::Mesh
    USkeletalMeshComponent* VehicleMesh = GetMesh();
    if (VehicleMesh && VehicleMesh->DoesSocketExist(ExitSocketName))
        Start = VehicleMesh->GetSocketLocation(ExitSocketName);

    FVector End = Start - FVector(0.0f, 0.0f, 500.0f);
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(DriverCharacter);

    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
        return Hit.ImpactPoint + FVector(0.0f, 0.0f, 100.0f);

    return Start + FVector(150.0f, 0.0f, 0.0f);
}

void ABaseVehicle::Input_Throttle(const FInputActionValue& Value)
{
    if (ChaosMovement) ChaosMovement->SetThrottleInput(Value.Get<float>());
}
void ABaseVehicle::Input_Steering(const FInputActionValue& Value)
{
    if (ChaosMovement) ChaosMovement->SetSteeringInput(Value.Get<float>());
}
void ABaseVehicle::Input_Brake(const FInputActionValue& Value)
{
    if (ChaosMovement) ChaosMovement->SetBrakeInput(Value.Get<float>());
}
void ABaseVehicle::Input_HandbrakeStarted(const FInputActionValue& Value)
{
    if (ChaosMovement) ChaosMovement->SetHandbrakeInput(true);
}
void ABaseVehicle::Input_HandbrakeStopped(const FInputActionValue& Value)
{
    if (ChaosMovement) ChaosMovement->SetHandbrakeInput(false);
}
void ABaseVehicle::Input_Look(const FInputActionValue& Value)
{
    const FInputActionValue::Axis2D LookAxis = Value.Get<FInputActionValue::Axis2D>();
    AddControllerYawInput(LookAxis.X);
    AddControllerPitchInput(LookAxis.Y);
}
