// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ASPickup.generated.h"

class UAbilitySystemComponent;
class USphereComponent;

UCLASS(Abstract)
class ARENASHOOTER_API AASPickup : public AActor
{
	GENERATED_BODY()
	
public:	
	AASPickup();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	// === Per-pickup policy (server only) ===
	virtual bool CanGiveTo(UAbilitySystemComponent* ASC);
	virtual bool GiveTo(AActor* Actor, UAbilitySystemComponent* ASC) PURE_VIRTUAL(AASPickup::GiveTo, return false;);

	void TryPickup(AActor* Actor);
	void Deactivate();
	void Respawn();

	UFUNCTION()
	void OnRep_IsActive();
	void ApplyActiveState();

	UPROPERTY(EditDefaultsOnly, Category="AS|Pickup")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(EditDefaultsOnly, Category="AS|Pickup")
	TObjectPtr<UStaticMeshComponent> PickupMesh;

	UPROPERTY(EditAnywhere, Category="AS|Pickup")
	bool bRespawns = true;

	UPROPERTY(EditAnywhere, Category="AS|Pickup", meta = (EditCondition = "bRespawns"))
	float RespawnTime = 15.f;

	UPROPERTY(ReplicatedUsing = OnRep_IsActive)
	bool bIsActive = true;

	FTimerHandle RespawnTimer;
};
