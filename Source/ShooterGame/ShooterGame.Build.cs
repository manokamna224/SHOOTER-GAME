using UnrealBuildTool;

public class ShooterGame : ModuleRules
{
    public ShooterGame(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Disable unity build to reduce peak memory usage on low-RAM machines
        bUseUnity = false;

        // Add the module root to include paths so ShooterGame.h is
        // resolvable from all subdirectories (Core/, Weapons/, AI/)
        PublicIncludePaths.AddRange(new string[]
        {
            "ShooterGame",
        });

        PrivateIncludePaths.AddRange(new string[]
        {
            "ShooterGame",
            "ShooterGame/Core",
            "ShooterGame/Weapons",
            "ShooterGame/AI",
        });

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "NavigationSystem",
            "GameplayAbilities",
            "GameplayTags",
            "GameplayTasks",
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",
            "SlateCore",
            "EnhancedInput",
            "ChaosVehicles",
            "PhysicsCore",
            "Chaos",
            "AIModule",
            "CommonUI",
            "DeveloperSettings",
            "Niagara",
        });
    }
}
