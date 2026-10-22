# ShooterGame - Open-World Shooter with Driving Mechanics

An Unreal Engine 5.8 C++ project following an iterative vertical-slice roadmap.

## Prerequisites

Before you can compile and run this project, you need:

1. **Unreal Engine 5.8** - Install via the Epic Games Launcher
2. **Visual Studio 2022** with the following workloads:
   - Desktop development with C++
   - Game development with C++
   - Unreal Engine installer (optional, can be added from UE installation)
3. **Git LFS** - Run `git lfs install` after installing Git

## Project Structure

```
ShooterGame/
├── ShooterGame.uproject          # Project descriptor (UE 5.8, plugins enabled)
├── .gitignore                    # Ignores Binaries/, Intermediate/, Saved/, etc.
├── .gitattributes                # Git LFS rules for .uasset, .umap, textures, meshes
├── README.md                     # This file
│
├── Config/
│   ├── DefaultEngine.ini         # Engine config (Lumen, Nanite, NavMesh, Chaos)
│   ├── DefaultGame.ini           # Game settings, asset manager config
│   └── DefaultInput.ini          # Enhanced Input axis defaults
│
└── Source/
    └── ShooterGame/
        ├── ShooterGame.Build.cs           # Module build dependencies (GAS, Chaos, Enhanced Input)
        ├── ShooterGame.Target.cs          # Standalone game target
        ├── ShooterGameEditor.Target.cs    # Editor target
        ├── ShooterGame.h / .cpp           # Module registration + log category
        │
        ├── Core/
        │   ├── ShooterGameInstance.h/.cpp         # Global game instance (cross-level state)
        │   ├── ShooterGameGameMode.h/.cpp          # GameMode (spawn rules, vehicle tracking)
        │   ├── ShooterGamePlayerController.h/.cpp  # Controller (IMC swap, enter/exit vehicle)
        │   ├── BaseCharacter.h/.cpp                # GAS character (movement, weapons, input)
        │   └── BaseVehicle.h/.cpp                 # Chaos vehicle (interaction, possession switch)
        │
        ├── Weapons/
        │   ├── BaseWeapon.h/.cpp                  # Modular weapon (hitscan + projectile firing)
        │   └── BaseProjectile.h/.cpp              # Ballistic projectile (gravity, drag, sub-stepping)
        │
        └── AI/
            └── ShooterGameAIController.h/.cpp      # Behavior tree AI (cover, fire, flank)
```

## What's Implemented (C++ Source)

### Phase 1: Environment Setup & Project Architecture
- [x] `.uproject` with UE 5.8, all required plugins enabled (GAS, Chaos Vehicles, Enhanced Input, World Partition, CommonUI)
- [x] `.gitignore` and `.gitattributes` with Git LFS tracking for binary assets
- [x] `Build.cs` with full module dependency list
- [x] Target files (game + editor)
- [x] `ShooterGameGameMode` - custom game mode with infantry/vehicle spawn logic
- [x] `ShooterGamePlayerController` - IMC swap, vehicle enter/exit interaction
- [x] `ABaseCharacter` - abstract character with GAS `AbilitySystemComponent`
- [x] `ABaseVehicle` - abstract vehicle inheriting from `AWheeledVehiclePawn`

### Phase 2: Core Movement & Weapon Architecture
- [x] Enhanced Input setup: `IA_Move`, `IA_Look`, `IA_Sprint`, `IA_Crouch`, `IA_Aim`, `IA_Fire`, `IA_Reload`
- [x] Movement speed logic: walk / sprint / aim-down-sights speed modifiers
- [x] `ABaseWeapon` - modular actor attaching to `hand_r_socket` with two firing modes:
  - **Hitscan** - line trace with distance-based damage falloff
  - **Projectile** - spawns `ABaseProjectile` with gravity + drag + sub-stepped movement
- [x] `ABaseProjectile` - ballistic simulation (gravity scale, drag coefficient, velocity stepping)
- [x] Physical material pass-through for impact effects (Blueprint hooks)

### Phase 5: Enemy AI (Core C++)
- [x] `AShooterGameAIController` - behavior tree runner with tactical state machine
- [x] Target acquisition and last-known-position tracking
- [x] Combat state enum: Idle, SeekingCover, InCover, Firing, Flanking, Reloading, Dead

## What Needs to Be Done in the Unreal Editor

The following items require the Unreal Editor (can't be done in C++ alone):

### Phase 1 (Editor Tasks)
- [ ] Create a Blank level named `OpenWorld` and enable World Partition
- [ ] Create Blueprint subclasses: `BP_PlayerCharacter`, `BP_EnemyCharacter`, `BP_SedanCar`
- [ ] Set the `PlayerControllerClass` in the GameMode Blueprint

### Phase 2 (Editor Tasks)
- [ ] Create Input Action assets: `IA_Move`, `IA_Look`, `IA_Sprint`, `IA_Crouch`, `IA_Aim`, `IA_Fire`, `IA_Reload`
- [ ] Create Input Mapping Contexts: `IMC_Infantry`, `IMC_Vehicle`
- [ ] Create weapon Blueprint subclasses: `BP_AssaultRifle`, `BP_SniperRifle`
- [ ] Create an Aim Offset blend space for 360-degree weapon aiming
- [ ] Create Physical Materials: `PM_Flesh`, `PM_Steel`, `PM_Glass`, `PM_Concrete`
- [ ] Assign physical materials to environment and character meshes

### Phase 3 (Editor Tasks)
- [ ] Rig vehicle mesh with bone hierarchy: `Root -> Wheel_FL, Wheel_FR, Wheel_RL, Wheel_RR`
- [ ] Configure `ChaosWheeledVehicleMovementComponent`: torque curves, gear ratios, suspension travel, tire friction
- [ ] Create vehicle wheel blueprint with Chaos wheel components
- [ ] Position `InteractionVolume` at the driver door in the vehicle Blueprint

### Phase 4 (Editor Tasks)
- [ ] Set up World Partition grid (64m or 128m cells)
- [ ] Configure Data Layers (roads, structures, dynamic props, foliage)
- [ ] Enable Lumen for GI and reflections
- [ ] Set up Post Process Volume with exposure clamping
- [ ] Enable Nanite on hard-surface environment assets

### Phase 5 (Editor Tasks)
- [ ] Create NavMeshBoundsVolume with dynamic generation
- [ ] Create Behavior Tree asset (`BT_EnemyCombat`) and Blackboard asset (`BB_EnemyCombat`)
- [ ] Set up EQS queries for cover selection and flanking
- [ ] Configure Mass AI or Spline Components for civilian traffic

### Phase 6 (Editor Tasks)
- [ ] Build MetaSound graphs for engine RPM / throttle / speed-driven audio
- [ ] Set up spatialized audio attenuation with occlusion
- [ ] Profile with Unreal Insights
- [ ] Package standalone development build via Project Launcher

## Getting Started

### 1. Install prerequisites
- Unreal Engine 5.8 (via Epic Games Launcher)
- Visual Studio 2022 with C++ game development workloads
- Git LFS: `git lfs install`

### 2. Generate project files
Right-click `ShooterGame.uproject` and select **Generate Visual Studio project files**.

### 3. Open and build
- Open `ShooterGame.sln` in Visual Studio
- Build in `Development Editor` configuration
- Or simply open `ShooterGame.uproject` directly

### 4. Create the first level
- In the editor, create a new Blank level
- Enable World Partition in the level settings
- Save as `Content/Maps/OpenWorld.umap`

## Architecture Notes

### Gameplay Ability System (GAS)
`ABaseCharacter` implements `IAbilitySystemInterface` and creates a `UAbilitySystemComponent`.
Starting abilities are granted on `PossessedBy()`. Subclasses define attribute sets
(health, shield, stamina) by creating `UAttributeSet` derivatives and registering them
in the constructor.

### Vehicle Possession Flow
```
On Foot:
  PlayerController -> possesses -> ABaseCharacter
  Active IMC: IMC_Infantry

Enter Vehicle (IA_Interact):
  1. PlayerController finds nearest ABaseVehicle within InteractRange
  2. ABaseVehicle::EnterVehicle() called
  3. Character mesh hidden + collision disabled
  4. Character attached to vehicle's DriverSeat socket
  5. Controller->UnPossess() then Controller->Possess(Vehicle)
  6. IMC swapped: IMC_Infantry -> IMC_Vehicle

Exit Vehicle (IA_Interact):
  1. ABaseVehicle::ExitVehicle() called
  2. Character detached from vehicle
  3. Character mesh visible + collision restored
  4. Safe exit location found via downward raycast from ExitPoint socket
  5. Character teleported to safe location
  6. Controller->UnPossess() then Controller->Possess(Character)
  7. IMC swapped: IMC_Vehicle -> IMC_Infantry
```

### Weapon Firing Pipeline
```
Hitscan:
  StartFire() -> Tick() fires at FireRateRPM interval
    -> FireShot() -> FireHitscan()
    -> Line trace from muzzle (aimed via camera)
    -> ComputeDamage(distance) with linear falloff
    -> ApplyPointDamage()
    -> BP_OnImpact(Hit, PhysMaterial)

Projectile:
  StartFire() -> Tick() fires at FireRateRPM interval
    -> FireShot() -> FireProjectile()
    -> SpawnActor<ABaseProjectile>()
    -> InitializeProjectile(velocity, damage, owner)
    -> Projectile::Tick() sub-steps:
       - Apply gravity (GravityScale)
       - Apply drag (DragCoefficient)
       - Sweep trace for impact
       -> HandleImpact() -> ApplyPointDamage() -> Destroy()
```

## Roadmap Status

| Phase | C++ Source | Editor Assets | Status |
|-------|-----------|---------------|--------|
| 1. Environment & Architecture | Done | Pending | **Scaffolded** |
| 2. Movement & Weapons | Done | Pending | **Scaffolded** |
| 3. Vehicle Simulation | Base classes done | Pending (mesh rig, curves) | **Scaffolded** |
| 4. World Construction | Config done | Pending (WP, Lumen, Nanite) | **Configured** |
| 5. Enemy AI | Core done | Pending (BT, BB, EQS) | **Scaffolded** |
| 6. Polish & Sound | Config done | Pending (MetaSounds) | **Not started** |
