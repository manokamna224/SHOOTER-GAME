// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "InputActionValue.h"
#include "BaseCharacter.generated.h"

// Forward declarations
class UInputAction;
class UInputMappingContext;
class UAttributeSet;
class ABaseWeapon;

/**
 * Base abstract character for the shooter game, equipped with a
 * Gameplay Ability System (GAS) AbilitySystemComponent.
 *
 * Features:
 *   - GAS integration (AbilitySystemComponent + AttributeSet)
 *   - Enhanced Input handling for movement, look, sprint, crouch, aim, fire, reload
 *   - Weapon attachment system (socket-based)
 *   - Health/shield attribute accessors
 *   - Camera + spring arm for first/third-person view
 *
 * Blueprint subclasses (e.g. BP_PlayerCharacter) configure the mesh,
 * animation blueprint, starting abilities, and input actions.
 */
UCLASS(Abstract)
class SHOOTERGAME_API ABaseCharacter : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    ABaseCharacter();

    // ─────────────────────────────────────────────────────────────
    //  ACharacter / AActor overrides
    // ─────────────────────────────────────────────────────────────
    virtual void PostInitializeComponents() override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_PlayerState() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // ─────────────────────────────────────────────────────────────
    //  IAbilitySystemInterface
    // ─────────────────────────────────────────────────────────────
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    // ─────────────────────────────────────────────────────────────
    //  Weapon management
    // ─────────────────────────────────────────────────────────────

    /** Equip a weapon to the character's right-hand socket */
    UFUNCTION(BlueprintCallable, Category = "ShooterGame|Weapons")
    virtual void EquipWeapon(ABaseWeapon* InWeapon);

    /** Drop or unequip the current weapon */
    UFUNCTION(BlueprintCallable, Category = "ShooterGame|Weapons")
    virtual void UnequipWeapon();

    /** Returns the currently equipped weapon (null if none) */
    UFUNCTION(BlueprintPure, Category = "ShooterGame|Weapons")
    FORCEINLINE ABaseWeapon* GetCurrentWeapon() const { return CurrentWeapon; }

    // ─────────────────────────────────────────────────────────────
    //  Combat state
    // ─────────────────────────────────────────────────────────────

    /** Whether the character is currently aiming down sights */
    UFUNCTION(BlueprintPure, Category = "ShooterGame|Combat")
    bool IsAiming() const { return bIsAiming; }

    /** Whether the character is currently sprinting */
    UFUNCTION(BlueprintPure, Category = "ShooterGame|Combat")
    bool IsSprinting() const { return bIsSprinting; }

    /** Toggle aim state (used by input) */
    UFUNCTION(BlueprintCallable, Category = "ShooterGame|Combat")
    void SetAiming(bool bNewAiming);

    /** Toggle sprint state (used by input) */
    UFUNCTION(BlueprintCallable, Category = "ShooterGame|Combat")
    void SetSprinting(bool bNewSprinting);

protected:
    // ─────────────────────────────────────────────────────────────
    //  GAS components
    // ─────────────────────────────────────────────────────────────

    /** The core GAS component that manages abilities and gameplay effects */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ShooterGame|Abilities")
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

    /** Default attribute set (health, shield, stamina, etc.) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ShooterGame|Abilities")
    TObjectPtr<UAttributeSet> AttributeSet;

    /** Gameplay abilities granted to this character on spawn */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Abilities")
    TArray<TSubclassOf<UGameplayAbility>> StartingAbilities;

    // ─────────────────────────────────────────────────────────────
    //  Camera
    // ─────────────────────────────────────────────────────────────

    /** Spring arm for third-person camera follow */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ShooterGame|Camera")
    TObjectPtr<class USpringArmComponent> CameraBoom;

    /** Third-person camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ShooterGame|Camera")
    TObjectPtr<class UCameraComponent> FollowCamera;

    // ─────────────────────────────────────────────────────────────
    //  Enhanced Input
    // ─────────────────────────────────────────────────────────────

    /** Movement input (2D vector: W/S/A/D or left stick) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Input")
    TObjectPtr<UInputAction> MoveAction;

    /** Look input (2D vector: mouse or right stick) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Input")
    TObjectPtr<UInputAction> LookAction;

    /** Sprint toggle (button) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Input")
    TObjectPtr<UInputAction> SprintAction;

    /** Crouch toggle (button) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Input")
    TObjectPtr<UInputAction> CrouchAction;

    /** Aim toggle (button) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Input")
    TObjectPtr<UInputAction> AimAction;

    /** Fire action (trigger for automatic, button for semi-auto) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Input")
    TObjectPtr<UInputAction> FireAction;

    /** Reload action (button) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Input")
    TObjectPtr<UInputAction> ReloadAction;

    // ─────────────────────────────────────────────────────────────
    //  Weapon system
    // ─────────────────────────────────────────────────────────────

    /** Socket name on the skeletal mesh where weapons attach */
    static FName WeaponSocketName;

    /** Currently equipped weapon actor */
    UPROPERTY(VisibleInstanceOnly, Replicated, BlueprintReadOnly, Category = "ShooterGame|Weapons")
    TObjectPtr<ABaseWeapon> CurrentWeapon;

    // ─────────────────────────────────────────────────────────────
    //  Movement state
    // ─────────────────────────────────────────────────────────────

    /** Base walk speed when not sprinting */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Movement")
    float WalkSpeed;

    /** Sprint speed multiplier */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Movement")
    float SprintSpeedMultiplier;

    /** Current aim state */
    bool bIsAiming;

    /** Current sprint state */
    bool bIsSprinting;

    // ─────────────────────────────────────────────────────────────
    //  Input handlers
    // ─────────────────────────────────────────────────────────────

    void Input_Move(const FInputActionValue& Value);
    void Input_Look(const FInputActionValue& Value);
    void Input_SprintStarted(const FInputActionValue& Value);
    void Input_SprintStopped(const FInputActionValue& Value);
    void Input_Crouch(const FInputActionValue& Value);
    void Input_AimStarted(const FInputActionValue& Value);
    void Input_AimStopped(const FInputActionValue& Value);
    void Input_FireStarted(const FInputActionValue& Value);
    void Input_FireStopped(const FInputActionValue& Value);
    void Input_Reload(const FInputActionValue& Value);

    // ─────────────────────────────────────────────────────────────
    //  Internal helpers
    // ─────────────────────────────────────────────────────────────

    /** Initialize GAS abilities from the StartingAbilities array */
    void GrantStartingAbilities();

    /** Apply walk vs sprint speed to the CharacterMovementComponent */
    void UpdateMovementSpeed();
};
