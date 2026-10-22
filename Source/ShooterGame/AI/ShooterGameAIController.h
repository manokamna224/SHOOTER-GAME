// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ShooterGameAIController.generated.h"

// Forward declarations
class UBehaviorTree;
class UBlackboardComponent;
class ABaseCharacter;

/**
 * Custom AI controller for tactical enemy combat behavior.
 *
 * Phase 5 behavior:
 *   - Runs a Behavior Tree with a Blackboard for state tracking
 *   - Tactical decision loop: Taking Cover -> Firing -> Flanking -> Reloading
 *   - Uses NavMesh dynamic generation for vehicle-aware pathing
 *   - Reports enemy position to the squad via Blackboard keys
 *
 * Blueprint subclass (BP_EnemyAIController) assigns:
 *   - Behavior tree asset
 *   - Blackboard asset
 *   - EQS (Environment Query System) queries for cover selection and flanking
 */
UCLASS()
class SHOOTERGAME_API AShooterGameAIController : public AAIController
{
    GENERATED_BODY()

public:
    AShooterGameAIController();

    virtual void OnPossess(APawn* InPawn) override;
    virtual void Tick(float DeltaSeconds) override;

protected:
    // ─────────────────────────────────────────────────────────────
    //  Behavior tree and blackboard
    // ─────────────────────────────────────────────────────────────

    /** Behavior tree to run for this AI */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShooterGame|AI")
    TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

    /** Blackboard keys used by the behavior tree */
    static FName TargetEnemyKey;
    static FName CurrentStateKey;
    static FName CoverLocationKey;
    static FName ShouldReloadKey;
    static FName LastKnownEnemyLocationKey;

    // ─────────────────────────────────────────────────────────────
    //  AI states (tracked in the blackboard)
    // ─────────────────────────────────────────────────────────────

    enum class EAICombatState : uint8
    {
        Idle,
        SeekingCover,
        InCover,
        Firing,
        Flanking,
        Reloading,
        Dead
    };

    /** Current combat state of this AI */
    EAICombatState CurrentCombatState;

protected:
    // ─────────────────────────────────────────────────────────────
    //  Perception
    // ─────────────────────────────────────────────────────────────

    /** Update the AI's target acquisition and state transitions */
    void UpdateCombatLogic(float DeltaSeconds);

    /** Find the nearest player pawn as a target */
    APawn* FindNearestTarget() const;

    /** Attempt to find a cover position near the AI using EQS (Blueprint hook) */
    UFUNCTION(BlueprintImplementableEvent, Category = "ShooterGame|AI", meta = (DisplayName = "Find Cover Location"))
    void BP_FindCoverLocation(const FVector& EnemyLocation, FVector& OutCoverLocation, bool& bFoundCover);

    /** Transition to a new combat state */
    void SetCombatState(EAICombatState NewState);
};
