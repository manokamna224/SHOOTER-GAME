// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

// ============================================================================
// ShooterGame module
// ----------------------------------------------------------------------------
// Open-world shooter with driving mechanics built on Unreal Engine 5.8.
//
// Module structure:
//   Core/     - GameMode, PlayerController, BaseCharacter, BaseVehicle
//   Weapons/  - BaseWeapon (hitscan + projectile), BaseProjectile
//   Input/    - Enhanced Input config and mapping context definitions
//   AI/       - Custom AIController for tactical combat behavior
//
// Plugins in use:
//   - Gameplay Ability System (GAS) for character abilities and weapon firing
//   - Chaos Vehicles for realistic vehicle physics simulation
//   - Enhanced Input for modular input mapping (infantry vs vehicle)
//   - World Partition for streaming large open-world levels
//   - CommonUI for menu and HUD infrastructure
// ============================================================================

DECLARE_LOG_CATEGORY_EXTERN(LogShooterGame, Log, All);
