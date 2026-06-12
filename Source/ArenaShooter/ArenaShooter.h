// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EASAbilityInputID : uint8
{
	// 0 None
	None				UMETA(DisplayName = "None"),
	// 1 Confirm
	Confirm				UMETA(DisplayName = "Confirm"),
	// 2 Cancel
	Cancel				UMETA(DisplayName = "Cancel"),
	// 3 Sprint
	Sprint				UMETA(DisplayName = "Sprint"),
	// 4 Jump
	Jump				UMETA(DisplayName = "Jump"),
	// 5 PrimaryFire
	PrimaryFire			UMETA(DisplayName = "Primary Fire"),
	// 6 SecondaryFire
	SecondaryFire		UMETA(DisplayName = "Secondary Fire"),
	// 7 Reload
	Reload				UMETA(DisplayName = "Reload"),
	// 8 NextWeapon
	NextWeapon			UMETA(DisplayName = "Next Weapon"), 
	// 9 PrevWeapon
	PrevWeapon			UMETA(DisplayName = "Previous Weapon")
};