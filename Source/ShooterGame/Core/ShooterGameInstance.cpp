#include "ShooterGameInstance.h"
#include "ShooterGame.h"

UShooterGameInstance::UShooterGameInstance()
{
    GameSettingsAsset = nullptr;
}

void UShooterGameInstance::Init()
{
    Super::Init();

    UE_LOG(LogShooterGame, Log, TEXT("ShooterGameInstance initialized."));
}

void UShooterGameInstance::Shutdown()
{
    UE_LOG(LogShooterGame, Log, TEXT("ShooterGameInstance shutting down."));

    Super::Shutdown();
}

void UShooterGameInstance::OnWorldPartitionLoaded()
{
    UE_LOG(LogShooterGame, Log, TEXT("World Partition level loaded."));
}
