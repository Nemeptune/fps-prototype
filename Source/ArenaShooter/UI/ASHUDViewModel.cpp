// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ASHUDViewModel.h"

void UASHUDViewModel::SetHealth(float Value)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(Health, Value))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthPercent);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthText);
	}
}

void UASHUDViewModel::SetMaxHealth(float Value)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(MaxHealth, Value))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthPercent);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthText);
	}
}

void UASHUDViewModel::SetShield(float Value)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(Shield, Value))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetShieldPercent);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetShieldText);
	}
}

void UASHUDViewModel::SetMaxShield(float Value)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(MaxShield, Value))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetShieldPercent);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetShieldText);
	}
}

void UASHUDViewModel::SetLocalKills(int32 Value)
{
	UE_MVVM_SET_PROPERTY_VALUE(LocalKills, Value);
}

void UASHUDViewModel::SetOpponentKills(int32 Value)
{
	UE_MVVM_SET_PROPERTY_VALUE(OpponentKills, Value);
}

void UASHUDViewModel::SetLocalName(const FText& Value)
{
	UE_MVVM_SET_PROPERTY_VALUE(LocalName, Value);
}

void UASHUDViewModel::SetOpponentName(const FText& Value)
{
	UE_MVVM_SET_PROPERTY_VALUE(OpponentName, Value);
}

void UASHUDViewModel::SetMatchTimeText(const FText& Value)
{
	UE_MVVM_SET_PROPERTY_VALUE(MatchTimeText, Value);
}

float UASHUDViewModel::GetHealthPercent() const
{
	return MaxHealth > 0.f ? Health / MaxHealth : 0.f;
}

float UASHUDViewModel::GetShieldPercent() const
{
	return MaxShield > 0.f ? Shield / MaxShield : 0.f;
}

FText UASHUDViewModel::GetHealthText() const
{
	return FText::FromString(FString::Printf(TEXT("%d/%d"), FMath::RoundToInt(Health), FMath::RoundToInt(MaxHealth)));
}

FText UASHUDViewModel::GetShieldText() const
{
	return FText::FromString(FString::Printf(TEXT("%d/%d"), FMath::RoundToInt(Shield), FMath::RoundToInt(MaxShield)));
}