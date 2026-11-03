#include "BaseProjectile.h"
#include "ShooterGame.h"
#include "BaseCharacter.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "Engine/DamageEvents.h"

ABaseProjectile::ABaseProjectile()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = false;

    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->InitSphereRadius(5.0f);
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block);
    CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    CollisionSphere->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    CollisionSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    CollisionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
    RootComponent = CollisionSphere;

    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
    ProjectileMesh->SetupAttachment(CollisionSphere);
    ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ProjectileMesh->SetVisibility(true);

    CollisionRadius = 5.0f;
    GravityScale = 1.0f;
    DragCoefficient = 0.001f;
    MaxRange = 500000.0f;

    ImpactDamage = 100.0f;
    Velocity = FVector::ZeroVector;
    TraveledDistance = 0.0f;
    OwningCharacter = nullptr;

    InitialLifeSpan = 10.0f;
}

void ABaseProjectile::BeginPlay()
{
    Super::BeginPlay();
}

void ABaseProjectile::InitializeProjectile(const FVector& InitialVelocity, float InDamage, ABaseCharacter* InOwner)
{
    Velocity = InitialVelocity;
    ImpactDamage = InDamage;
    OwningCharacter = InOwner;

    if (!Velocity.IsNearlyZero())
    {
        SetActorRotation(Velocity.Rotation());
    }
}

void ABaseProjectile::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (Velocity.IsNearlyZero())
    {
        Destroy();
        return;
    }

    const float MaxStepLength = 50.0f;
    const float StepLength = Velocity.Size() * DeltaSeconds;
    int32 NumSubSteps = FMath::CeilToInt(StepLength / MaxStepLength);
    NumSubSteps = FMath::Clamp(NumSubSteps, 1, 16);
    float SubDeltaTime = DeltaSeconds / NumSubSteps;

    for (int32 i = 0; i < NumSubSteps; ++i)
    {
        FVector GravityAccel = FVector(0.0f, 0.0f, GetWorld()->GetGravityZ() * GravityScale);
        Velocity += GravityAccel * SubDeltaTime;
        Velocity *= (1.0f - DragCoefficient * SubDeltaTime);

        FVector MoveDelta = Velocity * SubDeltaTime;
        float DeltaDistance = MoveDelta.Size();

        FVector Start = GetActorLocation();
        FVector End = Start + MoveDelta;

        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        QueryParams.AddIgnoredActor(OwningCharacter);
        QueryParams.bReturnPhysicalMaterial = true;
        QueryParams.bTraceComplex = true;

        FHitResult Hit;
        bool bHit = GetWorld()->SweepSingleByChannel(
            Hit, Start, End, FQuat::Identity,
            ECC_Visibility,
            FCollisionShape::MakeSphere(CollisionRadius),
            QueryParams
        );

        if (bHit)
        {
            HandleImpact(Hit);
            return;
        }

        SetActorLocation(End, false);
        TraveledDistance += DeltaDistance;

        if (TraveledDistance >= MaxRange)
        {
            Destroy();
            return;
        }
    }
}

void ABaseProjectile::HandleImpact(const FHitResult& Hit)
{
    if (Hit.GetActor())
    {
        UGameplayStatics::ApplyPointDamage(
            Hit.GetActor(),
            ImpactDamage,
            Hit.Normal,
            Hit,
            OwningCharacter ? OwningCharacter->GetController() : nullptr,
            OwningCharacter,
            UDamageType::StaticClass()
        );

        UPhysicalMaterial* HitMat = Hit.PhysMaterial.IsValid() ? Hit.PhysMaterial.Get() : nullptr;
        BP_OnImpact(Hit, HitMat);

        UE_LOG(LogShooterGame, Verbose, TEXT("Projectile hit %s at dist %.1f, dmg=%.1f"),
            *Hit.GetActor()->GetName(), TraveledDistance, ImpactDamage);
    }
    Destroy();
}

void ABaseProjectile::LifeSpanExpired()
{
    Destroy();
}
