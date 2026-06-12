// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ASAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "ASWeaponAttributeSet.generated.h"

struct FGameplayEffectModCallbackData;

/**
 * 
 */
UCLASS()
class ARENASHOOTER_API UASWeaponAttributeSet : public UASAttributeSet
{
	GENERATED_BODY()
public:
	UASWeaponAttributeSet();

	ATTRIBUTE_ACCESSORS(UASWeaponAttributeSet, Ammo);
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	UFUNCTION()
	void OnRep_Ammo(const FGameplayAttributeData& OldValue);

private:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Ammo, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Ammo;
};
