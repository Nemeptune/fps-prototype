// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySet.h"
#include "ASWeapon.generated.h"

class AASCharacter;
class UNiagaraSystem;

UCLASS()
class ARENASHOOTER_API AASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AASWeapon();

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "ArenaShooter|ASWeapon")
	FGameplayTag WeaponTag;
	
	void SetOwningCharacter(AASCharacter* InOwningCharacter);
	
	virtual void Equip();
	virtual void UnEquip();

	// === AbilitySet grant/remove ===
	virtual void AddAbilities();
	virtual void RemoveAbilities();

	// Persistent per-weapon ammo (the holstered store). The *live* value while equipped
	// is the Ammo attribute on the player's ASC; this just remembers the count when unequipped.
	int32 GetPersistentAmmo() const;
	void SetPersistentAmmo(int32 NewValue);
	void AddAmmo(int32 Value);
	
	UFUNCTION(BlueprintPure)
	FTransform GetMuzzleTransform(FName Socket = NAME_None) const;

	// === Components ===
	UFUNCTION(BlueprintPure)
	USkeletalMeshComponent*  GetWeaponMesh1P() const;
	UFUNCTION(BlueprintPure)
	USkeletalMeshComponent* GetWeaponMesh3P() const;

	UFUNCTION(BlueprintCallable, Category = "AS|Animation")
	UAnimMontage* GetEquip1PMontage() const;

	UFUNCTION(BlueprintCallable, Category = "AS|Animation")
	UAnimMontage* GetEquip3PMontage() const;
	
	const FSlateBrush& GetIcon() const;

	UFUNCTION(BlueprintCallable, Category = "AS|Animation")
	TSubclassOf<UAnimInstance> GetFirstPersonAnimLayer() const;
	
	UFUNCTION(BlueprintCallable, Category = "AS|Animation")
	TSubclassOf<UAnimInstance> GetThirdPersonAnimLayer() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AS|Fire")
	void Fire(const TArray<FVector>& impactPositions, const TArray<FVector>& impactNormals, const TArray<TEnumAsByte<EPhysicalSurface>>& impactSurfaceTypes);
	
protected:

	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

	UFUNCTION()
	void OnRep_PersistentAmmo();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AS|Weapon|Components")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh1P;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AS|Weapon|Components")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh3P;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AS|Animation")
	TSubclassOf<UAnimInstance> FirstPersonAnimLayer;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AS|Animation")
	TSubclassOf<UAnimInstance> ThirdPersonAnimLayer;

	//need custom character class for GetWeaponAttachPoint()
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "AS|Weapon|GSWeapon")
	TObjectPtr<AASCharacter> OwningCharacter;

	/** Data asset that defines what this weapon grants. */
	UPROPERTY(EditDefaultsOnly, Category="AS|Weapon|Abilities")
	TObjectPtr<UAbilitySet> AbilitySet = nullptr;
	
	/** Receipt of what this weapon granted so we can cleanly remove it on unequip. */
	UPROPERTY(VisibleInstanceOnly, Category="Abilities")
	FAbilitySet_GrantedHandles GrantedHandles;
	
	UPROPERTY(EditDefaultsOnly, Category ="AS|Weapon|Ammo")
	int32 StartingAmmo = 100;

	UPROPERTY(ReplicatedUsing = OnRep_PersistentAmmo)
	int32 PersistentAmmo = 0;
	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AS|Weapon|UI")
	FSlateBrush Icon;
	
	// === Mesh config ===
	UPROPERTY(EditDefaultsOnly, Category="AS|Weapon|Mesh")
	FName WeaponAttachPoint = TEXT("Weapon"); // socket on character mesh
	UPROPERTY(EditDefaultsOnly, Category="AS|Weapon|Mesh")
	FName MuzzleSocketName = TEXT("Muzzle"); 

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "AS|Animation")
	TObjectPtr<UAnimMontage> Equip1PMontage;
	UPROPERTY(BlueprintReadonly, EditAnywhere, Category = "AS|Animation")
	TObjectPtr<UAnimMontage> Equip3PMontage;

	UPROPERTY(BlueprintReadonly, EditAnywhere, Category = "AS|FX")
	TObjectPtr<UNiagaraSystem> MuzzleFlashSystem;
	UPROPERTY(BlueprintReadonly, EditAnywhere, Category = "AS|FX")
	TObjectPtr<UNiagaraSystem> ShellEjectSystem;
	UPROPERTY(BlueprintReadonly, EditAnywhere, Category = "AS|FX")
	TObjectPtr<UNiagaraSystem> TracerSystem;

	UPROPERTY(BlueprintReadWrite, Category = "AS|Fire")
	TArray<FVector> ImpactPositions;
	UPROPERTY(BlueprintReadWrite, Category = "AS|Fire")
	TArray<FVector> ImpactNormals;
	UPROPERTY(BlueprintReadWrite, Category = "AS|Fire")
	TArray<TEnumAsByte<EPhysicalSurface>> ImpactSurfaceTypes;

	UPROPERTY(BlueprintReadWrite, Category = "AS|Fire")
	FVector Muzzle1PPosition;
	UPROPERTY(BlueprintReadWrite, Category = "AS|Fire")
	FVector Muzzle3PPosition;
};
