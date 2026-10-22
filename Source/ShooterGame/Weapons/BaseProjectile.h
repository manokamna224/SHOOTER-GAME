// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseProjectile.generated.h"

// Forward declarations
class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class ABaseCharacter;

/**
 * Lightweight projectile actor for long-range / ballistic weapons.
 *
 * Simulates:
 *   - Gravity influence (configurable scale for gameplay tuning)
 *   - Air drag coefficient (velocity decay over distance)
 *   - Velocity-stepped movement (sub-stepped for accuracy at long range)
 *   - Impact detection with physical material handling
 *
 * Use with sniper rifles, launchers, and any weapon that benefits from
 * realistic ballistic drop rather than hitscan.
 *
 * Blueprint subclasses (e.g. BP_SniperRound, BP_Grenade) configure:
 *   - Mesh / trail effect
 *   - Impact effects per physical material
 *   - Explosion radius (for launchers)
 */
UCLASS(Abstract)
class SHOOTERGAME_API ABaseProjectile : public AActor
{
    GENERATED_BODY()

public:
    ABaseProjectile();

    // ─────────────────────────────────────────────────────────────
    //  AActor
    // ─────────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void LifeSpanExpired() override;

    // ─────────────────────────────────────────────────────────────
    //  Initialization
    // ─────────────────────────────────────────────────────────────

    /**
     * Initialize the projectile with initial velocity, damage, and owner.
     * Called immediately after spawn by ABaseWeapon::FireProjectile().
     */
    void InitializeProjectile(const FVector& InitialVelocity, float InDamage, ABaseCharacter* InOwner);

protected:
    // ─────────────────────────────────────────────────────────────
    //  Components
    // ─────────────────────────────────────────────────────────────

    /** Collision sphere for impact detection */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ShooterGame|Projectile|Components")
    TObjectPtr<USphereComponent> CollisionSphere;

    /** Visual mesh (bullet / tracer) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ShooterGame|Projectile|Components")
    TObjectPtr<UStaticMeshComponent> ProjectileMesh;

    // ─────────────────────────────────────────────────────────────
    //  Ballistic configuration
    // ─────────────────────────────────────────────────────────────

    /** Collision radius of the projectile */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Projectile|Config")
    float CollisionRadius;

    /** Gravity scale applied to the projectile (1.0 = real gravity) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Projectile|Config")
    float GravityScale;

    /** Drag coefficient - higher = more velocity loss over distance */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Projectile|Config")
    float DragCoefficient;

    /** Maximum travel distance before automatic destruction (cm) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|Projectile|Config")
    float MaxRange;

    /** Damage dealt on impact (set by InitializeProjectile) */
    float ImpactDamage;

    // ─────────────────────────────────────────────────────────────
    //  Runtime state
    // ─────────────────────────────────────────────────────────────

    /** Current velocity vector (cm/s) */
    FVector Velocity;

    /** Distance traveled so far */
    float TraveledDistance;

    /** Owning character for damage attribution */
    UPROPERTY()
    TObjectPtr<ABaseCharacter> OwningCharacter;

    // ─────────────────────────────────────────────────────────────
    //  Impact handling
    // ─────────────────────────────────────────────────────────────

    /** Handle collision impact - apply damage and destroy */
    void HandleImpact(const FHitResult& Hit);

    /** Blueprint hook for impact effects (particles, sound, decals) */
    UFUNCTION(BlueprintImplementableEvent, Category = "ShooterGame|Projectile", meta = (DisplayName = "On Impact"))
    void BP_OnImpact(const FHitResult& HitResult, class UPhysicalMaterial* HitMaterial);
};
