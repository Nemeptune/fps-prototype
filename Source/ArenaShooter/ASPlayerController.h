// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ASPlayerController.generated.h"

class UASResultWidget;
class UASAbilitySystemComponent;
struct FGameplayTag;
class UASInputConfig;
class UASHUDWidget;

/**
 * 
 */
UCLASS()
class ARENASHOOTER_API AASPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	void SetAbilitySystemComponent(UASAbilitySystemComponent* ASC);
	
	void CreateHUD();
	UASHUDWidget* GetHUD();

	TSubclassOf<UASResultWidget> GetResultWidgetClass() const;
	
protected:
	UPROPERTY()
	TObjectPtr<UASAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UASInputConfig> InputConfig;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UI")
	TSubclassOf<UASHUDWidget> UIHUDWidgetClass;
	UPROPERTY(BlueprintReadWrite, Category = "UI")
	TObjectPtr<UASHUDWidget> UIHUDWidget;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UASResultWidget> ResultWidgetClass;
	// Server only
	virtual void OnPossess(APawn *InPawn) override;

	virtual void OnRep_PlayerState() override;

	void AbilityInputPressed(FGameplayTag InputTag);
	void AbilityInputReleased(FGameplayTag InputTag);

};
