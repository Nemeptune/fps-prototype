// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/ASPickup_Ammo.h"
#include "ASCharacter.h"
#include "ASWeapon.h"

bool AASPickup_Ammo::GiveTo(AActor* Actor, UAbilitySystemComponent* ASC)
{
	AASCharacter* Character = Cast<AASCharacter>(Actor);
	if (!Character)
	{
		return false;
	}

	AASWeapon* Target = WeaponTag.IsValid() ? Character->FindWeaponByTag(WeaponTag) : Character->GetCurrentWeapon();
	if (!Target)
	{
		return false;
	}

	Target->AddAmmo(AmmoAmount);
	return true;
}
