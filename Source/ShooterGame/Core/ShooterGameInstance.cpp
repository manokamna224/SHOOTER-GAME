#include "ShooterGameInstance.h"
#include "ShooterGame.h"

AShooterGameInstance::AShooterGameInstance()
{
    GameSettingsAsset = nullptr;
}

void AShooterGameInstance::Init()
{
    Super::Init();

    UE_LOG(LogShooterGame, Log, TEXT("ShooterGameInstance initialized."));
}

void AShooterGameInstance::Shutdown()
{
    UE_LOG(LogShooterGame, Log, TEXT("ShooterGameInstance shutting down."));

    Super::Shutdown();
}

void AShooterGameInstance::OnWorldPartitionLoaded()
{
    UE_LOG(LogShooterGame, Log, TEXT("World Partition level loaded."));
}
