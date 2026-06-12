// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "ASUIManager.generated.h"

class UASResultWidget;
struct FOnAttributeChangeData;
class AASPlayerController;
class AASPlayerState;
class UASHUDViewModel;
class AASCharacter;
class AASWeapon;
class UASWeaponSlotViewModel;
class UASHUDWidget;
class UASAbilitySystemComponent;
class AASGameState;

/**
 * 
 */
UCLASS()
class ARENASHOOTER_API UASUIManager : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
public:
	virtual void Deinitialize() override;

	void OnLocalPlayerReady(AASPlayerController* PC, TSubclassOf<UUserWidget> HUDClass);
	

private:
	
	void EnsureHUD(AASPlayerController* PC, TSubclassOf<UUserWidget> HUDClass);
	void AssignViewModel();
	void BindLocalPlayerState();
	void BindWorldState();
	void BindPawn(AASPlayerController* PC);
	void ResetState();

	void HandlePlayerAdded(APlayerState* PlayerState);
	void HandlePlayerRemoved(APlayerState* PlayerState);

	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandleMaxHealthChanged(const FOnAttributeChangeData& Data);
	void HandleShieldChanged(const FOnAttributeChangeData& Data);
	void HandleMaxShieldChanged(const FOnAttributeChangeData& Data);
	void HandleAmmoChanged(const FOnAttributeChangeData& Data);
	void HandleCurrentWeaponChanged();
	void HandleMatchEnded();
	UFUNCTION()
	void HandleLocalNameChanged();
	UFUNCTION()
	void HandleOpponentNameChanged();
	UFUNCTION()
	void HandlePawnChanged(APawn* OldPawn, APawn* NewPawn);

	UFUNCTION()
	void HandleLocalKills(int32 NewKills);
	UFUNCTION()
	void HandleOpponentKills(int32 NewKills);

	void InitHotBar();
	void RefreshHotbar();
	void UpdateTimer();
	int32 GetCurrentAmmo() const;
	
	UPROPERTY()
	TObjectPtr<UASHUDViewModel> HUDVM;
	UPROPERTY()
	TObjectPtr<UASHUDWidget> HUDWidget;
	UPROPERTY()
	TSubclassOf<UASResultWidget> ResultWidgetClass;
	UPROPERTY()
	TObjectPtr<UASResultWidget> ResultWidget;
	UPROPERTY()
	TObjectPtr<AASPlayerState> LocalPS;
	UPROPERTY()
	TObjectPtr<AASPlayerState> OpponentPS;
	UPROPERTY()
	TObjectPtr<AASCharacter> LocalCharacter;
	UPROPERTY()
	TArray<TObjectPtr<UASWeaponSlotViewModel>> SlotsVM;
	TWeakObjectPtr<UASAbilitySystemComponent> BoundASC;
	TWeakObjectPtr<AASGameState> BoundGS;

	int32 ActiveSlot = INDEX_NONE;

	FTimerHandle Timer;
	bool bLocalBound = false;
	bool bWorldBound = false;
	bool bPawnBound = false;
};
