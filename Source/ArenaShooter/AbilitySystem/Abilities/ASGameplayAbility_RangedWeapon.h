// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ASGameplayAbility.h"
#include "ASGameplayAbility_RangedWeapon.generated.h"

#define COLLISION_PROJECTILE ECC_GameTraceChannel1

/**
 * 
 */
UCLASS()
class ARENASHOOTER_API UASGameplayAbility_RangedWeapon : public UASGameplayAbility
{
	GENERATED_BODY()

public:
	UASGameplayAbility_RangedWeapon();

	//~UGameplayAbility interface
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~End of UGameplayAbility interface

protected:
	// Damage effect to apply
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// Range of trace
	UPROPERTY(EditDefaultsOnly, Category="Ability|Trace")
	float MaxRange = 10000.f;

	void PerformLocalTargeting(OUT FHitResult& OutHits);
	
	void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag);
	
	UFUNCTION(BlueprintCallable)
	void StartRangedWeaponTargeting();
	
	// Called when target data is ready
	UFUNCTION(BlueprintImplementableEvent)
	void OnRangedWeaponTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetData);
private:
	FDelegateHandle OnTargetDataReadyCallbackDelegateHandle;

	UPROPERTY(EditDefaultsOnly, Category="Ability|Visuals", meta=(AllowPrivateAccess = "true"))
	FColor LineColor = FColor::Red;
};
