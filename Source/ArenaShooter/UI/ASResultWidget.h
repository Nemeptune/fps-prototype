// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ASResultWidget.generated.h"

/**
 * 
 */
UCLASS()
class ARENASHOOTER_API UASResultWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void InitResult(bool bWon, bool bDraw);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void OnResultSet(bool bWon, bool bDraw);

	UFUNCTION(BlueprintCallable)
	void ReturnToMainMenu();

	UPROPERTY(EditDefaultsOnly, Category = "AS|Result")
	TSoftObjectPtr<UWorld> MainMenuLevel;
};
