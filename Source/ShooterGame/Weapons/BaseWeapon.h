// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseWeapon.generated.h"

// Forward declarations
class ABaseCharacter;
class UAbilitySystemComponent;
class ABaseProjectile;
class UPhysicalMaterial;

/**
 * Firing modes supported by the weapon system.
 *   Hitscan   - Line trace with distance-based damage falloff (automatic weapons, CQ)
 *   Projectile- Lightweight actor with gravity + drag simulation (sniper rifles, launchers)
 */
UENUM(BlueprintType)
enum class EWeaponFireMode : uint8
{
    Hitscan        UMETA(DisplayName = "Hitscan (Line Trace)"),
    Projectile     UMETA(DisplayName = "Projectile (Ballistic)"),
};

/**
 * FBaseWeapon - Modular weapon actor attached to the character's skeletal mesh socket.
 *
 * Design:
 *   - Weapon is a separate Actor, not a component, for clean replication and physics
 *   - Two firing models:
 *       Hitscan:  FHitResult line trace -> damage with falloff
 *       Projectile: Spawns ABaseProjectile with gravity, drag, velocity steps
 *   - Physical material mapping governs impact effects and penetration math
 *   - Firing can be automatic (hold) or semi-auto (tap)
 *
 * Blueprint subclasses (e.g. BP_AssaultRifle, BP_SniperRifle) configure:
 *   - Mesh, muzzle socket, fire rate, damage, ammo
 *   - Projectile class (for projectile mode)
 *   - Impact effects per physical material
 */
UCLASS(Abstract)
class SHOOTERGAME_API ABaseWeapon : public AActor
{
    GENERATED_BODY()

public:
    ABaseWeapon();

    // ─────────────────────────────────────────────────────────────
    //  AActor
    // ─────────────────────────────────────────────────────────────
    virtual void BeginPlay() override;

    // ─────────────────────────────────────────────────────────────
    //  Firing control
    // ─────────────────────────────────────────────────────────────

    /** Start firing (called on input press) */
    virtual void StartFire();

    /** Stop firing (called on input release) */
    virtual void StopFire();

    /** Reload the weapon */
    virtual void Reload();

    // ─────────────────────────────────────────────────────────────
    //  Weapon properties
    // ─────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintPure, Category = "ShooterGame|Weapon")
    FORCEINLINE EWeaponFireMode GetFireMode() const { return FireMode; }

    UFUNCTION(BlueprintPure, Category = "ShooterGame|Weapon")
    FORCEINLINE float GetDamage() const { return BaseDamage; }

    UFUNCTION(BlueprintPure, Category = "ShooterGame|Weapon")
    FORCEINLINE int32 GetCurrentAmmo() const { return CurrentAmmo; }

    UFUNCTION(BlueprintPure, Category = "ShooterGame|Weapon")
    FORCEINLINE int32 GetMaxAmmo() const { return MaxAmmo; }

    UFUNCTION(BlueprintPure, Category = "ShooterGame|Weapon")
    bool IsReloading() const { return bIsReloading; }

    /** Set the owning character (called during equip/unequip) */
    void SetOwningCharacter(ABaseCharacter* InCharacter) { OwningCharacter = InCharacter; }

    UFUNCTION(BlueprintPure, Category = "ShooterGame|Weapon")
    ABaseCharacter* GetOwningCharacter() const { return OwningCharacter; }

protected:
    // ─────────────────────────────────────────────────────────────
    //  Weapon configuration
    // ─────────────────────────────────────────────────────────────

    /** Skeletal mesh for the weapon visual */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ShooterGame|Weapon|Components")
    TObjectPtr<class USkeletalMeshComponent> WeaponMesh;

    /** Firing mode: hitscan line trace or projectile spawn */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Weapon|Config")
    EWeaponFireMode FireMode;

    /** Base damage before falloff and material modifiers */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Weapon|Config")
    float BaseDamage;

    /** Damage falloff start distance (cm) - full damage within this range */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Weapon|Config")
    float FalloffStartDistance;

    /** Damage falloff end distance (cm) - minimum damage beyond this range */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Weapon|Config")
    float FalloffEndDistance;

    /** Minimum damage multiplier at max falloff distance */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Weapon|Config")
    float MinDamageMultiplier;

    /** Fire rate in rounds per minute */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Weapon|Config")
    float FireRateRPM;

    /** Whether the weapon fires automatically while the trigger is held */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Weapon|Config")
    bool bIsAutomatic;

    /** Magazine capacity */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Weapon|Config")
    int32 MaxAmmo;

    /** Reload time in seconds */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Weapon|Config")
    float ReloadTime;

    /** Muzzle socket name on the weapon mesh */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Weapon|Config")
    FName MuzzleSocketName;

    /** Projectile class to spawn when FireMode is Projectile */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Weapon|Config")
    TSubclassOf<ABaseProjectile> ProjectileClass;

    /** Initial projectile speed in cm/s (projectile mode only) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Weapon|Config")
    float ProjectileMuzzleVelocity;

    // ─────────────────────────────────────────────────────────────
    //  Runtime state
    // ─────────────────────────────────────────────────────────────

    /** Current rounds in magazine */
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "ShooterGame|Weapon|State")
    int32 CurrentAmmo;

    /** Is the trigger currently held */
    bool bWantsToFire;

    /** Is a reload in progress */
    bool bIsReloading;

    /** Time accumulator for fire rate timing */
    float FireTimer;

    /** Time accumulator for reload timing */
    float ReloadTimer;

    /** Owning character reference */
    UPROPERTY()
    TObjectPtr<ABaseCharacter> OwningCharacter;

    // ─────────────────────────────────────────────────────────────
    //  Firing logic
    // ─────────────────────────────────────────────────────────────

    /** Perform a single shot (hitscan or projectile) */
    virtual void FireShot();

    /** Hitscan line trace from the muzzle, applying damage falloff */
    void FireHitscan();

    /** Spawn a projectile actor from the muzzle */
    void FireProjectile();

    /** Compute damage with distance falloff applied */
    float ComputeDamage(float Distance) const;

    /** Get the muzzle world location and rotation */
    void GetMuzzleTransform(FVector& OutLocation, FRotator& OutRotation) const;

    /** Handle post-fire ammo decrement */
    void ConsumeAmmo();

    /** Blueprint hook for muzzle flash / recoil effects */
    UFUNCTION(BlueprintImplementableEvent, Category = "ShooterGame|Weapon", meta = (DisplayName = "On Fire"))
    void BP_OnFire();

    /** Blueprint hook for impact effects (particles, decals, sound) */
    UFUNCTION(BlueprintImplementableEvent, Category = "ShooterGame|Weapon", meta = (DisplayName = "On Impact"))
    void BP_OnImpact(const FHitResult& HitResult, UPhysicalMaterial* HitMaterial);

    /** Blueprint hook for reload start/end */
    UFUNCTION(BlueprintImplementableEvent, Category = "ShooterGame|Weapon", meta = (DisplayName = "On Reload Start"))
    void BP_OnReloadStart();
    UFUNCTION(BlueprintImplementableEvent, Category = "ShooterGame|Weapon", meta = (DisplayName = "On Reload Complete"))
    void BP_OnReloadComplete();

    /** Get the fire rate as seconds between shots */
    float GetFireInterval() const;
};
