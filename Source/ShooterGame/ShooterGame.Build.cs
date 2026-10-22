using UnrealBuildTool;

public class ShooterGame : ModuleRules
{
    public ShooterGame(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Public engine module dependencies
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "NavigationSystem",
        });

        // Private engine module dependencies
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",
            "SlateCore",
            "GameplayTags",
            "GameplayTasks",
            "EnhancedInput",
            "ChaosVehicles",
            "ChaosVehiclesEngine",
            "ChaosVehiclesData",
            "CommonUI",
            "DeveloperSettings",
            "AIModule",
            "AudioMixer",
            "PhysicsCore",
            "Niagara",
            "Chaos",
            "FieldSystemCore",
            "FieldSystemEngine",
        });

        // Gameplay Ability System (GAS) - weapons, abilities, attribute sets
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "GameplayAbilities",
        });

        // For UAbilitySystemComponent and related GAS types
        PrivateDependencyModuleNames.Add("GameplayAbilities");
    }
}
