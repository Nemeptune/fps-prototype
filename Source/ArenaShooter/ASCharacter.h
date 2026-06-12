// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "ASCharacter.generated.h"


class AASWeapon;
class UASCombatAttributeSet;
class UASAbilitySystemComponent;
class UGameplayEffect;
class UASGameplayAbility;
class UCameraComponent;
class USpringArmComponent;

DECLARE_MULTICAST_DELEGATE(FOnInventoryChangedSignature);
DECLARE_MULTICAST_DELEGATE(FOnCurrentWeaponChangedSignature);

USTRUCT()
struct FReplicatedWeaponState
{
	GENERATED_BODY()

	UPROPERTY() int32 SlotIndex = -1;   // index into Inventory
	UPROPERTY() uint8 Seq = 0;          // which client request produced this
};

USTRUCT()
struct FReplicatedDeathState
{
	GENERATED_BODY()

	UPROPERTY()
	bool bIsDead = false;

	UPROPERTY()
	FVector_NetQuantizeNormal ImpulseDir = FVector::ZeroVector;

	UPROPERTY()
	FVector_NetQuantize10 ImpulseLocation = FVector::ZeroVector;

	UPROPERTY()
	FName ImpulseBone = NAME_None;
};

USTRUCT(BlueprintType)
struct FWeaponAnimLayers
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag WeaponTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> FPLinkedLayer = nullptr;
 
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> TPLinkedLayer = nullptr;
};

UCLASS()
class ARENASHOOTER_API AASCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	// ===========================================================
	//  Construction & engine lifecycle
	// ===========================================================

	AASCharacter(const FObjectInitializer& ObjectInitializer);
	virtual void PossessedBy(AController* NewController) override; 	// Only called on the Server. Calls before Server's AcknowledgePossession.
	virtual void OnRep_PlayerState() override; 	// Client only
	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override; 	// Implement IAbilitySystemInterface
	
protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	
	// ===========================================================
	//  Ability System & Attributes
	// ===========================================================

	UFUNCTION(BlueprintCallable, Category = "ASCharacter")
	int32 GetAbilityLevel() const;

	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "ArenaShooter|ASCharacter|Attributes")
	float GetHealth() const;
	UFUNCTION(BlueprintCallable, Category = "ArenaShooter|ASCharacter|Attributes")
	float GetMaxHealth() const;
	UFUNCTION(BlueprintCallable, Category = "ArenaShooter|ASCharacter|Attributes")
	float GetShield() const;
	UFUNCTION(BlueprintCallable, Category = "ArenaShooter|ASCharacter|Attributes")
	float GetMaxShield() const;

	void RemoveCharacterAbilities(); // Server

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ArenaShooter|Abilities")
	TObjectPtr<UASAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ArenaShooter|Abilities")
	TObjectPtr<UASCombatAttributeSet> CombatAttributes;

	// Default abilities for this Character. These will be removed on Character death and regiven if Character respawns.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ArenaShooter|Abilities")
	TArray<TSubclassOf<UASGameplayAbility>> CharacterAbilities;

	// Ability that coordinates death. Granted once and kept across respawns (it must survive the
	// stripping of CharacterAbilities). Triggered by the Event.Death gameplay event.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ArenaShooter|Abilities")
	TSubclassOf<UASGameplayAbility> DeathAbility;
	
	// This is an instant GE that overrides the values for attributes that get reset on spawn/respawn.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ArenaShooter|Abilities")
	TSubclassOf<UGameplayEffect> DefaultAttributes;

	// Grant abilities on the Server. The Ability Specs will be replicated to the owning client.
	virtual void AddCharacterAbilities();

	// Grants the (persistent) death ability on the Server, once per ASC.
	void AddDeathAbility();
	virtual void InitializeAttributes();

	// Only for respawn — otherwise change attributes via a GE. These set the base value.
	virtual void SetHealth(float Health);
	virtual void SetShield(float Health);

public:
	// ===========================================================
	//  Inventory & Weapon switching
	// ===========================================================

	UFUNCTION(BlueprintCallable, Category = "ArenaShooter|Inventory")
	AASWeapon* GetCurrentWeapon() const;

	UFUNCTION(BlueprintCallable, Category = "ArenaShooter|Inventory")
	int32 GetNumWeapons() const; // for GA Next/Previous Weapon

	UFUNCTION(BlueprintCallable, Category = "ArenaShooter|Inventory")
	bool AddWeaponToInventory(AASWeapon* NewWeapon, bool bEquipWeapon = false);

	UFUNCTION(BlueprintCallable, Category = "ArenaShooter|Inventory")
	bool RemoveWeaponFromInventory(AASWeapon* WeaponToRemove);

	UFUNCTION(BlueprintCallable, Category = "ArenaShooter|Inventory")
	void RemoveAllWeaponsFromInventory();

	UFUNCTION(BlueprintCallable, Category = "ArenaShooter|Inventory")
	void EquipWeapon(AASWeapon* NewWeapon);

	UFUNCTION(BlueprintCallable, Category = "ArenaShooter|Inventory")
	virtual void NextWeapon();

	UFUNCTION(BlueprintCallable, Category = "ArenaShooter|Inventory")
	virtual void PreviousWeapon();

	// Owning-client entry point: predict + ask server. Use this from input.
	void RequestSwitch(int32 NewSlot);
	const TArray<TObjectPtr<AASWeapon>>& GetInventory() const { return Inventory; }
	UFUNCTION(BlueprintCallable, Category = "ArenaShooter|Inventory")
	int32 GetActiveSlotIndex() const;

	FTimerHandle EquipLockTimerHandle;
	FOnCurrentWeaponChangedSignature OnCurrentWeaponChanged;
	FOnInventoryChangedSignature OnInventoryChanged;
	
	USkeletalMeshComponent* GetMesh1P() const;
 
	FName GetWeapon1PAttachPoint() const;
	FName GetWeapon3PAttachPoint() const;

	UFUNCTION(BlueprintCallable, Category = "ArenaShooter|Inventory")
	AASWeapon* FindWeaponByTag(FGameplayTag Tag) const;

protected:

	UPROPERTY(ReplicatedUsing = OnRep_Inventory)
	TArray<TObjectPtr<AASWeapon>> Inventory;

	// Authoritative active-weapon selection. Replicated to EVERYONE (owner included).
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedWeaponState)
	FReplicatedWeaponState ReplicatedWeaponState;

	// Owner-local prediction state (not replicated).
	int32 PredictedSlot = -1;
	uint8 LocalSeq = 0;

	UPROPERTY()
	TObjectPtr<AASWeapon> CurrentWeapon;

	FGameplayTag CurrentWeaponTag;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "ArenaShooter|Inventory")
	TArray<TSubclassOf<AASWeapon>> DefaultInventoryClasses;

	// --- Switching flow ---
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetActiveWeapon(int32 NewSlot, uint8 Seq);

	// Server-initiated equip (spawn/respawn default, pickups). Authority only.
	void SetActiveWeaponSlotAuthoritative(int32 NewSlot);

	UFUNCTION()
	void OnRep_ReplicatedWeaponState();

	// Maps a slot to the weapon and drives the existing equip path. Safe if slot invalid.
	void ApplyWeaponVisual(int32 Slot);

	// --- Equip mechanics ---
	void SetCurrentWeapon(AASWeapon* NewWeapon, AASWeapon* LastWeapon);
	void UnEquipWeapon(AASWeapon* WeaponToUnEquip); // Unequips the specified weapon
	void UnEquipCurrentWeapon(); // Unequips the current weapon.
	void SelectBestLayer(const AASWeapon* Weapon); 	// Choose layer according to weapon tag

	// --- Inventory mechanics ---
	void SpawnDefaultInventory(); 	// Server
	bool DoesWeaponExistInInventory(AASWeapon* InWeapon);
	UFUNCTION()
	void OnRep_Inventory();

	// ===========================================================
	//  Death & ragdoll
	// ===========================================================
public:
	void StartDeath(const FVector& ImpulseDir, const FVector& ImpulseLocation, FName ImpulseBone); // Server

protected:
	
	UPROPERTY(EditDefaultsOnly, Category = "ArenaShooter|Death")
	float DeathImpulseStrength = 30000.f;
	void ApplyDeathImpulse();
	void SetRagdollPhysics(); // Runs on server and on every client via OnRep_DeathState.
	// Replicated to everyone so simulated proxies (and late/lagging clients) ragdoll on catch-up.
	UPROPERTY(ReplicatedUsing = OnRep_DeathState)
	FReplicatedDeathState DeathState;
	UFUNCTION()
	void OnRep_DeathState();
	
private:
	
	// ===========================================================
	//  Components
	// ===========================================================

	/** pawn mesh: 1st person view */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ArenaShooter|Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh1P;
	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ArenaShooter|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ArenaShooter|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArm;

	// ===========================================================
	//  Weapon attach sockets & anim layers
	// ===========================================================

	UPROPERTY(EditDefaultsOnly, Category="ArenaShooter|Weapon|Attach", meta=(GetOptions="Get1PSockets"))
	FName WeaponAttachSocket1P = NAME_None;

	UPROPERTY(EditDefaultsOnly, Category="ArenaShooter|Weapon|Attach", meta=(GetOptions="Get3PSockets"))
	FName WeaponAttachSocket3P = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ArenaShooter|Animation", meta = (AllowPrivateAccess = "true"))
	TArray<FWeaponAnimLayers> WeaponAnimLayers;

	UPROPERTY(Transient)
	TSubclassOf<UAnimInstance> CurrentFirstPersonLayerClass;
	UPROPERTY(Transient)
	TSubclassOf<UAnimInstance> CurrentThirdPersonLayerClass;

	UFUNCTION()
	TArray<FName> Get1PSockets() const;
	UFUNCTION()
	TArray<FName> Get3PSockets() const;

	// ===========================================================
	//  Input
	// ===========================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ArenaShooter|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ArenaShooter|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ArenaShooter|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ArenaShooter|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ArenaShooter|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> TurnAction;

	UFUNCTION()
	void Move(const FInputActionValue& Value);
	UFUNCTION()
	void LookUp(const FInputActionValue& Value);
	UFUNCTION()
	void Turn(const FInputActionValue& Value);
	
};
