#include "BaseCharacter.h"
#include "ShooterGame.h"
#include "Weapons/BaseWeapon.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

// Socket name for weapon attachment on the character's skeletal mesh
FName ABaseCharacter::WeaponSocketName = TEXT("hand_r_socket");

ABaseCharacter::ABaseCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    // GAS component
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

    // Default attribute set reference (subclasses create a specific AttributeSet subclass)
    // We leave this null in the base; derived classes assign a UAttributeSet derivative.
    AttributeSet = nullptr;

    // Camera
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->SocketOffset = FVector(0.0f, 60.0f, 50.0f);

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // Movement defaults
    WalkSpeed = 600.0f;
    SprintSpeedMultiplier = 1.6f;

    bIsAiming = false;
    bIsSprinting = false;

    CurrentWeapon = nullptr;

    // Input actions are set in Blueprint defaults
    MoveAction = nullptr;
    LookAction = nullptr;
    SprintAction = nullptr;
    CrouchAction = nullptr;
    AimAction = nullptr;
    FireAction = nullptr;
    ReloadAction = nullptr;
}

void ABaseCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();
}

void ABaseCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    // Initialize GAS for the server (authority)
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->InitAbilityActorInfo(this, this, this);
    }

    GrantStartingAbilities();
}

void ABaseCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    // Initialize GAS for the client
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->InitAbilityActorInfo(this, this, this);
    }
}

void ABaseCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void ABaseCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    UpdateMovementSpeed();
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(ABaseCharacter, CurrentWeapon, COND_None);
}

UAbilitySystemComponent* ABaseCharacter::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void ABaseCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!EnhancedInput)
    {
        UE_LOG(LogShooterGame, Error, TEXT("BaseCharacter: InputComponent is not EnhancedInputComponent."));
        return;
    }

    // Movement
    if (MoveAction)
    {
        EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);
    }

    // Look
    if (LookAction)
    {
        EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Look);
    }

    // Sprint
    if (SprintAction)
    {
        EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &ThisClass::Input_SprintStarted);
        EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &ThisClass::Input_SprintStopped);
    }

    // Crouch
    if (CrouchAction)
    {
        EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Started, this, &ThisClass::Input_Crouch);
    }

    // Aim
    if (AimAction)
    {
        EnhancedInput->BindAction(AimAction, ETriggerEvent::Started, this, &ThisClass::Input_AimStarted);
        EnhancedInput->BindAction(AimAction, ETriggerEvent::Completed, this, &ThisClass::Input_AimStopped);
    }

    // Fire
    if (FireAction)
    {
        EnhancedInput->BindAction(FireAction, ETriggerEvent::Started, this, &ThisClass::Input_FireStarted);
        EnhancedInput->BindAction(FireAction, ETriggerEvent::Completed, this, &ThisClass::Input_FireStopped);
    }

    // Reload
    if (ReloadAction)
    {
        EnhancedInput->BindAction(ReloadAction, ETriggerEvent::Started, this, &ThisClass::Input_Reload);
    }
}

// ═══════════════════════════════════════════════════════════════
//  Weapon management
// ═══════════════════════════════════════════════════════════════

void ABaseCharacter::EquipWeapon(ABaseWeapon* InWeapon)
{
    if (!InWeapon)
    {
        return;
    }

    // Unequip existing weapon first
    if (CurrentWeapon)
    {
        UnequipWeapon();
    }

    CurrentWeapon = InWeapon;

    // Attach the weapon to the character's hand socket
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (MeshComp)
    {
        InWeapon->AttachToComponent(
            MeshComp,
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            WeaponSocketName
        );
    }

    InWeapon->SetOwningCharacter(this);

    UE_LOG(LogShooterGame, Log, TEXT("Equipped weapon: %s"), *InWeapon->GetName());
}

void ABaseCharacter::UnequipWeapon()
{
    if (!CurrentWeapon)
    {
        return;
    }

    CurrentWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    CurrentWeapon->SetOwningCharacter(nullptr);

    UE_LOG(LogShooterGame, Log, TEXT("Unequipped weapon: %s"), *CurrentWeapon->GetName());

    CurrentWeapon = nullptr;
}

// ═══════════════════════════════════════════════════════════════
//  Combat state
// ═══════════════════════════════════════════════════════════════

void ABaseCharacter::SetAiming(bool bNewAiming)
{
    if (bIsAiming == bNewAiming)
    {
        return;
    }

    bIsAiming = bNewAiming;
    UpdateMovementSpeed();
}

void ABaseCharacter::SetSprinting(bool bNewSprinting)
{
    if (bIsSprinting == bNewSprinting)
    {
        return;
    }

    bIsSprinting = bNewSprinting;
    UpdateMovementSpeed();
}

// ═══════════════════════════════════════════════════════════════
//  Input handlers
// ═══════════════════════════════════════════════════════════════

void ABaseCharacter::Input_Move(const FInputActionValue& Value)
{
    // 2D axis: Y = forward/back, X = right/left
    const FInputActionValue::Axis2D MoveAxis = Value.Get<FInputActionValue::Axis2D>();
    const float Forward = MoveAxis.Y;
    const float Right = MoveAxis.X;

    if (Controller)
    {
        // Forward movement relative to controller yaw
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

        const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

        AddMovementInput(ForwardDir, Forward);
        AddMovementInput(RightDir, Right);
    }
}

void ABaseCharacter::Input_Look(const FInputActionValue& Value)
{
    const FInputActionValue::Axis2D LookAxis = Value.Get<FInputActionValue::Axis2D>();
    AddControllerYawInput(LookAxis.X);
    AddControllerPitchInput(LookAxis.Y);
}

void ABaseCharacter::Input_SprintStarted(const FInputActionValue& Value)
{
    SetSprinting(true);
}

void ABaseCharacter::Input_SprintStopped(const FInputActionValue& Value)
{
    SetSprinting(false);
}

void ABaseCharacter::Input_Crouch(const FInputActionValue& Value)
{
    if (bIsCrouched)
    {
        UnCrouch();
    }
    else
    {
        Crouch();
    }
}

void ABaseCharacter::Input_AimStarted(const FInputActionValue& Value)
{
    SetAiming(true);
}

void ABaseCharacter::Input_AimStopped(const FInputActionValue& Value)
{
    SetAiming(false);
}

void ABaseCharacter::Input_FireStarted(const FInputActionValue& Value)
{
    if (CurrentWeapon)
    {
        CurrentWeapon->StartFire();
    }
}

void ABaseCharacter::Input_FireStopped(const FInputActionValue& Value)
{
    if (CurrentWeapon)
    {
        CurrentWeapon->StopFire();
    }
}

void ABaseCharacter::Input_Reload(const FInputActionValue& Value)
{
    if (CurrentWeapon)
    {
        CurrentWeapon->Reload();
    }
}

// ═══════════════════════════════════════════════════════════════
//  Internal helpers
// ═══════════════════════════════════════════════════════════════

void ABaseCharacter::GrantStartingAbilities()
{
    if (!AbilitySystemComponent || !HasAuthority())
    {
        return;
    }

    for (const TSubclassOf<UGameplayAbility>& AbilityClass : StartingAbilities)
    {
        if (AbilityClass)
        {
            FGameplayAbilitySpec Spec(AbilityClass, 1, static_cast<int32>(AbilityClass.GetDefaultObject()->GetInputID()), this);
            AbilitySystemComponent->GiveAbility(Spec);
        }
    }
}

void ABaseCharacter::UpdateMovementSpeed()
{
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!MoveComp)
    {
        return;
    }

    float TargetSpeed = WalkSpeed;

    // Sprinting multiplies base speed, but not while aiming
    if (bIsSprinting && !bIsAiming)
    {
        TargetSpeed *= SprintSpeedMultiplier;
    }

    // Aiming slows the character down (ADS speed penalty)
    if (bIsAiming)
    {
        TargetSpeed *= 0.5f;
    }

    MoveComp->MaxWalkSpeed = TargetSpeed;
}
