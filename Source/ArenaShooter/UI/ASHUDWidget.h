// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ASHUDWidget.generated.h"

class UASWeaponSlotViewModel;

/**
 * 
 */
UCLASS()
class ARENASHOOTER_API UASHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void InitWeaponSlots(const TArray<UASWeaponSlotViewModel*>& Slots);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 NumWeaponSlots = 2;
};
