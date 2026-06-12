// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "ASHUDViewModel.generated.h"

class UASWeaponSlotViewModel;

/**
 * 
 */
UCLASS()
class ARENASHOOTER_API UASHUDViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()
public:

	// --- Setters ---
	void SetHealth(float Value);
	void SetMaxHealth(float Value);
	void SetShield(float Value);
	void SetMaxShield(float Value);
	void SetLocalKills(int32 Value);
	void SetOpponentKills(int32 Value);
	void SetLocalName(const FText& Value);
	void SetOpponentName(const FText& Value);
	void SetMatchTimeText(const FText& Value);

	UFUNCTION(BlueprintPure, FieldNotify)
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, FieldNotify)
	float GetShieldPercent() const;

	UFUNCTION(BlueprintPure, FieldNotify)
	FText GetHealthText() const;
	
	UFUNCTION(BlueprintPure, FieldNotify)
	FText GetShieldText() const;

private:
	UPROPERTY(BlueprintReadOnly, FieldNotify, meta=(AllowPrivateAccess))
	float Health = 0.f;
	UPROPERTY(BlueprintReadOnly, FieldNotify, meta=(AllowPrivateAccess))
	float MaxHealth = 0.f;
	UPROPERTY(BlueprintReadOnly, FieldNotify, meta=(AllowPrivateAccess))
	float Shield = 0.f;
	UPROPERTY(BlueprintReadOnly, FieldNotify, meta=(AllowPrivateAccess))
	float MaxShield = 0.f;
	UPROPERTY(BlueprintReadOnly, FieldNotify, meta=(AllowPrivateAccess))
	int32 LocalKills = 0;
	UPROPERTY(BlueprintReadOnly, FieldNotify, meta=(AllowPrivateAccess))
	int32 OpponentKills = 0;
	UPROPERTY(BlueprintReadOnly, FieldNotify, meta=(AllowPrivateAccess))
	FText LocalName;
	UPROPERTY(BlueprintReadOnly, FieldNotify, meta=(AllowPrivateAccess))
	FText OpponentName;
	UPROPERTY(BlueprintReadOnly, FieldNotify, meta=(AllowPrivateAccess))
	FText MatchTimeText;
};
