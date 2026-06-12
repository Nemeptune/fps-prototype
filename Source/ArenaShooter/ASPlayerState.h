// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "ASPlayerState.generated.h"

class UASCombatAttributeSet;
class UASWeaponAttributeSet;
class UASAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKillsChanged, int32, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeathsChanged, int32, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerNameChanged);

/**
 * 
 */
UCLASS()
class ARENASHOOTER_API AASPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	AASPlayerState();

	UFUNCTION(BlueprintCallable, Category = "ASPlayerState")
	UASAbilitySystemComponent* GetASAbilityComponent() const {return AbilitySystemComponent;}
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UASCombatAttributeSet* GetCombatAttributeSet() const;
	UASWeaponAttributeSet* GetWeaponAttributeSet() const;

	UFUNCTION(BlueprintCallable, Category = "ASPlayerState|Attributes")
	float GetHealth() const;
	UFUNCTION(BlueprintCallable, Category = "ASPlayerState|Attributes")
	float GetMaxHealth() const;
	UFUNCTION(BlueprintCallable, Category = "ASPlayerState|Attributes")
	float GetShield() const;
	UFUNCTION(BlueprintCallable, Category = "ASPlayerState|Attributes")
	float GetMaxShield() const;

	int32 GetKills() const;
	int32 GetDeaths() const;

	UPROPERTY(BlueprintAssignable, Category="ASPlayerState|Score")
	FOnKillsChanged OnKillsChanged;

	UPROPERTY(BlueprintAssignable, Category="ASPlayerState|Score")
	FOnDeathsChanged OnDeathsChanged;

	UPROPERTY(BlueprintAssignable, Category="ASPlayerState")
	FOnPlayerNameChanged OnNameChanged;

	UFUNCTION(BlueprintCallable)
	void AddKill();
	UFUNCTION(BlueprintCallable)
	void AddDeath();
	
protected:
	
	UPROPERTY(VisibleAnywhere, Category = "PlayerState")
	TObjectPtr<UASAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UASCombatAttributeSet> CombatAttributes;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UASWeaponAttributeSet> WeaponAttributes;

	UPROPERTY(BlueprintReadOnly, Transient, ReplicatedUsing=OnRep_Kills, Category = "ASPlayerState|Score")
	int32 Kills = 0;
	UPROPERTY(BlueprintReadOnly, Transient, ReplicatedUsing=OnRep_Deaths, Category = "ASPlayerState|Score")
	int32 Deaths = 0;

	UFUNCTION()
	void OnRep_Kills();
	UFUNCTION()
	void OnRep_Deaths();
	virtual void OnRep_PlayerName() override;
};
