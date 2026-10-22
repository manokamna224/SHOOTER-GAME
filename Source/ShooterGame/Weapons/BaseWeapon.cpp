#include "BaseWeapon.h"
#include "ShooterGame.h"
#include "Core/BaseCharacter.h"
#include "BaseProjectile.h"

#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Engine/DamageEvents.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"

ABaseWeapon::ABaseWeapon()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    // Weapon mesh (set the skeletal mesh asset in Blueprint subclasses)
    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    RootComponent = WeaponMesh;

    // Defaults
    FireMode = EWeaponFireMode::Hitscan;
    BaseDamage = 25.0f;
    FalloffStartDistance = 1000.0f;   // 10m
    FalloffEndDistance = 20000.0f;     // 200m
    MinDamageMultiplier = 0.3f;
    FireRateRPM = 600.0f;
    bIsAutomatic = true;
    MaxAmmo = 30;
    ReloadTime = 2.5f;
    MuzzleSocketName = TEXT("Muzzle");
    ProjectileClass = nullptr;
    ProjectileMuzzleVelocity = 30000.0f; // 300 m/s

    CurrentAmmo = MaxAmmo;
    bWantsToFire = false;
    bIsReloading = false;
    FireTimer = 0.0f;
    ReloadTimer = 0.0f;
    OwningCharacter = nullptr;
}

void ABaseWeapon::BeginPlay()
{
    Super::BeginPlay();

    CurrentAmmo = MaxAmmo;
}

void ABaseWeapon::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // Handle fire rate timing for automatic fire
    if (FireTimer > 0.0f)
    {
        FireTimer -= DeltaSeconds;
    }

    if (bWantsToFire && !bIsReloading)
    {
        if (FireTimer <= 0.0f && CurrentAmmo > 0)
        {
            FireShot();
            FireTimer = GetFireInterval();

            // For semi-auto, reset bWantsToFire after a single shot
            if (!bIsAutomatic)
            {
                bWantsToFire = false;
            }
        }
    }

    // Handle reload timing
    if (bIsReloading)
    {
        ReloadTimer -= DeltaSeconds;
        if (ReloadTimer <= 0.0f)
        {
            bIsReloading = false;
            CurrentAmmo = MaxAmmo;
            BP_OnReloadComplete();
            UE_LOG(LogShooterGame, Log, TEXT("Reload complete. Ammo: %d/%d"), CurrentAmmo, MaxAmmo);
        }
    }
}

void ABaseWeapon::StartFire()
{
    if (bIsReloading)
    {
        return;
    }

    if (CurrentAmmo <= 0)
    {
        // Auto-reload when out of ammo
        Reload();
        return;
    }

    bWantsToFire = true;

    // Fire immediately on press (resets fire timer)
    if (FireTimer <= 0.0f)
    {
        FireShot();
        FireTimer = GetFireInterval();

        if (!bIsAutomatic)
        {
            bWantsToFire = false;
        }
    }
}

void ABaseWeapon::StopFire()
{
    bWantsToFire = false;
}

void ABaseWeapon::Reload()
{
    if (bIsReloading || CurrentAmmo >= MaxAmmo)
    {
        return;
    }

    bIsReloading = true;
    ReloadTimer = ReloadTime;
    bWantsToFire = false;

    BP_OnReloadStart();
    UE_LOG(LogShooterGame, Log, TEXT("Reloading..."));
}

void ABaseWeapon::FireShot()
{
    ConsumeAmmo();
    BP_OnFire();

    switch (FireMode)
    {
    case EWeaponFireMode::Hitscan:
        FireHitscan();
        break;

    case EWeaponFireMode::Projectile:
        FireProjectile();
        break;
    }
}

void ABaseWeapon::FireHitscan()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Get muzzle position
    FVector MuzzleLocation;
    FRotator MuzzleRotation;
    GetMuzzleTransform(MuzzleLocation, MuzzleRotation);

    // Aim from the camera for accurate targeting, not from the muzzle
    FVector AimStart = MuzzleLocation;
    FVector AimDir = MuzzleRotation.Vector();

    // Use the owning controller's view for aiming precision
    if (OwningCharacter && OwningCharacter->GetController())
    {
        APlayerController* PC = Cast<APlayerController>(OwningCharacter->GetController());
        if (PC)
        {
            FVector CamLoc;
            FRotator CamRot;
            PC->GetPlayerViewPoint(CamLoc, CamRot);

            AimStart = MuzzleLocation;
            AimDir = CamRot.Vector();
        }
    }

    // Trace endpoint
    FVector TraceEnd = AimStart + (AimDir * FalloffEndDistance * 1.2f);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredActor(OwningCharacter);
    QueryParams.bReturnPhysicalMaterial = true;
    QueryParams.bTraceComplex = true; // Hit individual triangles for precision

    FHitResult Hit;
    bool bHit = World->LineTraceSingleByChannel(Hit, AimStart, TraceEnd, ECC_Visibility, QueryParams);

    FVector EndPoint = bHit ? Hit.ImpactPoint : TraceEnd;

    // Draw debug tracer (remove in production)
    // DrawDebugLine(World, MuzzleLocation, EndPoint, FColor::Red, false, 0.1f, 0, 1.0f);

    if (bHit && Hit.GetActor())
    {
        float Distance = Hit.Distance;
        float FinalDamage = ComputeDamage(Distance);

        // Apply damage
        UGameplayStatics::ApplyPointDamage(
            Hit.GetActor(),
            FinalDamage,
            AimDir,
            Hit,
            OwningCharacter ? OwningCharacter->GetController() : nullptr,
            this,
            UDamageType::StaticClass()
        );

        // Notify Blueprint for impact effects
        UPhysicalMaterial* HitMat = Hit.PhysMaterial.IsValid() ? Hit.PhysMaterial.Get() : nullptr;
        BP_OnImpact(Hit, HitMat);

        UE_LOG(LogShooterGame, Verbose, TEXT("Hitscan hit %s at %.1f cm, damage=%.1f"),
            *Hit.GetActor()->GetName(), Distance, FinalDamage);
    }
}

void ABaseWeapon::FireProjectile()
{
    UWorld* World = GetWorld();
    if (!World || !ProjectileClass)
    {
        UE_LOG(LogShooterGame, Warning, TEXT("FireProjectile: No projectile class set."));
        return;
    }

    FVector MuzzleLocation;
    FRotator MuzzleRotation;
    GetMuzzleTransform(MuzzleLocation, MuzzleRotation);

    // Spawn params
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = OwningCharacter ? Cast<APawn>(OwningCharacter) : nullptr;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // Spawn the projectile
    ABaseProjectile* Projectile = World->SpawnActor<ABaseProjectile>(
        ProjectileClass,
        MuzzleLocation,
        MuzzleRotation,
        SpawnParams
    );

    if (Projectile)
    {
        // Set initial velocity
        FVector Velocity = MuzzleRotation.Vector() * ProjectileMuzzleVelocity;
        Projectile->InitializeProjectile(Velocity, BaseDamage, OwningCharacter);
    }
}

float ABaseWeapon::ComputeDamage(float Distance) const
{
    if (Distance <= FalloffStartDistance)
    {
        return BaseDamage;
    }

    if (Distance >= FalloffEndDistance)
    {
        return BaseDamage * MinDamageMultiplier;
    }

    // Linear interpolation between full and minimum damage
    float Alpha = (Distance - FalloffStartDistance) / (FalloffEndDistance - FalloffStartDistance);
    return FMath::Lerp(BaseDamage, BaseDamage * MinDamageMultiplier, Alpha);
}

void ABaseWeapon::GetMuzzleTransform(FVector& OutLocation, FRotator& OutRotation) const
{
    if (WeaponMesh && WeaponMesh->DoesSocketExist(MuzzleSocketName))
    {
        OutLocation = WeaponMesh->GetSocketLocation(MuzzleSocketName);
        OutRotation = WeaponMesh->GetSocketRotation(MuzzleSocketName);
    }
    else
    {
        // Fallback: use actor location if muzzle socket is missing
        OutLocation = GetActorLocation();
        OutRotation = GetActorRotation();
    }
}

void ABaseWeapon::ConsumeAmmo()
{
    if (CurrentAmmo > 0)
    {
        CurrentAmmo--;
    }

    if (CurrentAmmo <= 0)
    {
        bWantsToFire = false;
    }
}

float ABaseWeapon::GetFireInterval() const
{
    if (FireRateRPM <= 0.0f)
    {
        return 0.1f;
    }

    return 60.0f / FireRateRPM;
}
