// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ASWeaponAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UASWeaponAttributeSet::UASWeaponAttributeSet()
{
}

void UASWeaponAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UASWeaponAttributeSet, Ammo, COND_OwnerOnly, REPNOTIFY_Always);
}

void UASWeaponAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	if (Attribute == GetAmmoAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

void UASWeaponAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	if (Attribute == GetAmmoAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

void UASWeaponAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	if (Data.EvaluatedData.Attribute == GetAmmoAttribute())
	{
		SetAmmo(FMath::Max(GetAmmo(), 0.0f));
	}
}

void UASWeaponAttributeSet::OnRep_Ammo(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UASWeaponAttributeSet, Ammo, OldValue);
}
