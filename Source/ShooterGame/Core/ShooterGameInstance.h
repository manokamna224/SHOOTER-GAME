// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ShooterGameInstance.generated.h"

/**
 * Custom GameInstance for the shooter game.
 * Manages global state, persistent subsystem initialization, and
 * cross-level data that must survive level transitions.
 */
UCLASS()
class SHOOTERGAME_API AShooterGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    AShooterGameInstance();

protected:
    virtual void Init() override;
    virtual void Shutdown() override;

public:
    /** Called when a level is loaded to support World Partition streaming callbacks */
    virtual void OnWorldPartitionLoaded();

protected:
    /** Global settings asset reference for runtime gameplay tuning */
    UPROPERTY(EditDefaultsOnly, Category = "ShooterGame")
    TObjectPtr<UObject> GameSettingsAsset;
};
