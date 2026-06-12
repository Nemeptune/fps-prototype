#include "ASGameplayTags.h"

// Abilities
UE_DEFINE_GAMEPLAY_TAG(FASGameplayTags::Ability_Weapon,			"Ability.Weapon");
UE_DEFINE_GAMEPLAY_TAG(FASGameplayTags::Ability_Melee,			"Ability.Melee");

// Weapon state
UE_DEFINE_GAMEPLAY_TAG(FASGameplayTags::Weapon_IsFiring,		"Weapon.IsFiring");

// Weapon type
UE_DEFINE_GAMEPLAY_TAG(FASGameplayTags::Weapon_None,			"Weapon.None");
UE_DEFINE_GAMEPLAY_TAG(FASGameplayTags::Weapon_Pistol,			"Weapon.Pistol");
UE_DEFINE_GAMEPLAY_TAG(FASGameplayTags::Weapon_Rifle,			"Weapon.Rifle");
UE_DEFINE_GAMEPLAY_TAG(FASGameplayTags::Weapon_RocketLauncher,	"Weapon.RocketLauncher");

UE_DEFINE_GAMEPLAY_TAG(FASGameplayTags::Status_Death, "Status.Death");
UE_DEFINE_GAMEPLAY_TAG(FASGameplayTags::Status_Death_Dying, "Status.Death.Dying");
UE_DEFINE_GAMEPLAY_TAG(FASGameplayTags::Status_Death_Dead, "Status.Death.Dead");

// Gameplay events that drive the death/respawn ability
UE_DEFINE_GAMEPLAY_TAG(FASGameplayTags::Event_Death, "Event.Death");
UE_DEFINE_GAMEPLAY_TAG(FASGameplayTags::Event_Respawn, "Event.Character.Respawned");