#include "ShooterGameAIController.h"
#include "ShooterGame.h"
#include "Core/BaseCharacter.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

// Blackboard key names
FName AShooterGameAIController::TargetEnemyKey = TEXT("TargetEnemy");
FName AShooterGameAIController::CurrentStateKey = TEXT("CombatState");
FName AShooterGameAIController::CoverLocationKey = TEXT("CoverLocation");
FName AShooterGameAIController::ShouldReloadKey = TEXT("ShouldReload");
FName AShooterGameAIController::LastKnownEnemyLocationKey = TEXT("LastKnownEnemyLocation");

AShooterGameAIController::AShooterGameAIController()
{
    CurrentCombatState = EAICombatState::Idle;
    BehaviorTreeAsset = nullptr;
}

void AShooterGameAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (!InPawn)
    {
        return;
    }

    // Run the behavior tree
    if (BehaviorTreeAsset)
    {
        RunBehaviorTree(BehaviorTreeAsset);

        UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
        if (BlackboardComp)
        {
            // Initialize blackboard keys
            BlackboardComp->SetValueAsObject(TargetEnemyKey, nullptr);
            BlackboardComp->SetValueAsEnum(CurrentStateKey, static_cast<uint8>(EAICombatState::Idle));
            BlackboardComp->SetValueAsBool(ShouldReloadKey, false);
        }

        UE_LOG(LogShooterGame, Log, TEXT("AIController possessed %s, running behavior tree."),
            *InPawn->GetName());
    }
    else
    {
        UE_LOG(LogShooterGame, Warning, TEXT("AIController: No behavior tree asset assigned."));
    }
}

void AShooterGameAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    UpdateCombatLogic(DeltaSeconds);
}

void AShooterGameAIController::UpdateCombatLogic(float DeltaSeconds)
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
    {
        return;
    }

    UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
    if (!BlackboardComp)
    {
        return;
    }

    // Simple target acquisition loop
    AActor* CurrentTarget = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetEnemyKey));

    if (!CurrentTarget)
    {
        // Find nearest enemy
        APawn* NearestEnemy = FindNearestTarget();
        if (NearestEnemy)
        {
            BlackboardComp->SetValueAsObject(TargetEnemyKey, NearestEnemy);
            BlackboardComp->SetValueAsVector(LastKnownEnemyLocationKey, NearestEnemy->GetActorLocation());
            SetCombatState(EAICombatState::SeekingCover);

            UE_LOG(LogShooterGame, Verbose, TEXT("AIController found target: %s"), *NearestEnemy->GetName());
        }
    }
    else
    {
        // Update last known enemy position
        BlackboardComp->SetValueAsVector(LastKnownEnemyLocationKey, CurrentTarget->GetActorLocation());

        // Check if the target is dead or too far
        ABaseCharacter* TargetCharacter = Cast<ABaseCharacter>(CurrentTarget);
        if (TargetCharacter && TargetCharacter->IsPendingKill())
        {
            BlackboardComp->SetValueAsObject(TargetEnemyKey, nullptr);
            SetCombatState(EAICombatState::Idle);
        }
    }
}

APawn* AShooterGameAIController::FindNearestTarget() const
{
    // Find all player-controlled characters
    TArray<AActor*> Players;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseCharacter::StaticClass(), Players);

    APawn* NearestPlayer = nullptr;
    float NearestDistSq = MAX_FLT;

    FVector MyLocation = GetPawn()->GetActorLocation();

    for (AActor* Actor : Players)
    {
        // Skip AI-controlled characters (only target players)
        ABaseCharacter* Char = Cast<ABaseCharacter>(Actor);
        if (!Char || !Char->IsPlayerControlled())
        {
            continue;
        }

        float DistSq = FVector::DistSquared2D(MyLocation, Actor->GetActorLocation());
        if (DistSq < NearestDistSq)
        {
            NearestDistSq = DistSq;
            NearestPlayer = Cast<APawn>(Actor);
        }
    }

    return NearestPlayer;
}

void AShooterGameAIController::SetCombatState(EAICombatState NewState)
{
    if (CurrentCombatState == NewState)
    {
        return;
    }

    CurrentCombatState = NewState;

    UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsEnum(CurrentStateKey, static_cast<uint8>(NewState));
    }
}
