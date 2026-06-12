// Fill out your copyright notice in the Description page of Project Settings.


#include "ASCharacter.h"

#include "AbilitySystem/ASAbilitySystemComponent.h"
#include "ASCombatAttributeSet.h"
#include "ASGameplayTags.h"
#include "ASLogChannels.h"
#include "AbilitySystem/Abilities/ASGameplayAbility.h"
#include "ASPlayerController.h"
#include "ASPlayerState.h"
#include "Components/ASCharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ASWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Sound/SoundCue.h"
#include "AbilitySystem/ASWeaponAttributeSet.h"

//FName AASCharacter::AbilitySystemComponentName(TEXT("ASGAS"));

AASCharacter::AASCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UASCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->SetRelativeLocation(FVector(0, 50, 68.492264));
	SpringArm->bUsePawnControlRotation = false;

	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(RootComponent);
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh1P"));
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh1P->SetCollisionProfileName(FName("NoCollision"));
	Mesh1P->bReceivesDecals = false;
	Mesh1P->CastShadow = false;

	CurrentWeaponTag = FASGameplayTags::Weapon_None;
}

//server only
void AASCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	AASPlayerState* PS = GetPlayerState<AASPlayerState>();
	if (PS)
	{
		AbilitySystemComponent = PS->GetASAbilityComponent();
		check(AbilitySystemComponent);

		// For AI characters who don't have PlayerController. For others initting twice with no harm
		AbilitySystemComponent->InitAbilityActorInfo(PS, this);

		CombatAttributes = PS->GetCombatAttributeSet();
		InitializeAttributes();

		// Set Health/Shield to their max. This is only necessary for *Respawn*.
		SetHealth(GetMaxHealth());
		SetShield(GetMaxShield());

		AddCharacterAbilities();
		AddDeathAbility();
		AASPlayerController* PC = Cast<AASPlayerController>(GetController());
		if (PC)
		{
			// Setting asc in pc for tagged inputs
			PC->SetAbilitySystemComponent(AbilitySystemComponent);
			PC->CreateHUD();
		}
	}
}

void AASCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	AASPlayerState* PS = GetPlayerState<AASPlayerState>();
	if (PS)
	{
		AbilitySystemComponent = PS->GetASAbilityComponent();

		AbilitySystemComponent->InitAbilityActorInfo(PS, this);

		CombatAttributes = PS->GetCombatAttributeSet();
		
		// If we handle players disconnecting and rejoining in the future, we'll have to change this so that posession from rejoining doesn't reset attributes.
		// For now assume possession = spawn/respawn.
		InitializeAttributes();

		AASPlayerController* PC = Cast<AASPlayerController>(GetController());
		if (PC)
		{
			// Setting asc in pc for tagged inputs
			PC->SetAbilitySystemComponent(AbilitySystemComponent);
			PC->CreateHUD();
		}

		// Set Health/Shield to their max. This is only necessary for *Respawn*.
		SetHealth(GetMaxHealth());
		SetShield(GetMaxShield());
	}
}

void AASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AASCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (GetMesh() && GetCharacterMovement())
	{
		GetMesh()->AddTickPrerequisiteComponent(GetCharacterMovement());
	}
	
	GetWorldTimerManager().SetTimerForNextTick(this, &ThisClass::SpawnDefaultInventory);
}

void AASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	const APlayerController* PC = GetController<APlayerController>();
	const ULocalPlayer* LP = PC->GetLocalPlayer();

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

	Subsystem->ClearAllMappings();

	Subsystem->AddMappingContext(DefaultMappingContext, 0);
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AASCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AASCharacter::LookUp);
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &AASCharacter::Turn);
	}
}

void AASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AASCharacter, Inventory);
	DOREPLIFETIME(AASCharacter, ReplicatedWeaponState);
	DOREPLIFETIME(AASCharacter, DeathState);
}

UAbilitySystemComponent* AASCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AASCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AASCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(CurrentWeaponTag);
		CurrentWeaponTag = FASGameplayTags::Weapon_None;
		AbilitySystemComponent->AddLooseGameplayTag(CurrentWeaponTag);
	}
	
	Super::EndPlay(EndPlayReason);
}

int32 AASCharacter::GetAbilityLevel() const
{
	return 1;
}

bool AASCharacter::IsAlive() const
{
	return GetHealth() > 0.0f;
}

float AASCharacter::GetHealth() const
{
	if (CombatAttributes)
	{
		return CombatAttributes->GetHealth();
	}
	
	return 0.0f;
}

float AASCharacter::GetMaxHealth() const
{
	if (CombatAttributes)
	{
		return CombatAttributes->GetMaxHealth();
	}
	
	return 0.0f;
}

float AASCharacter::GetShield() const
{
	if (CombatAttributes)
	{
		return CombatAttributes->GetShield();
	}
	
	return 0.0f;
}

float AASCharacter::GetMaxShield() const
{
	if (CombatAttributes)
	{
		return CombatAttributes->GetMaxShield();
	}
	
	return 0.0f;
}

void AASCharacter::RemoveCharacterAbilities()
{
	if (GetLocalRole() < ROLE_Authority || !IsValid(AbilitySystemComponent) || !AbilitySystemComponent->bCharacterAbilitiesGiven)
	{
		return;
	}

	TArray<FGameplayAbilitySpecHandle> AbilitiesToRemove;
	for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (Spec.SourceObject == this && CharacterAbilities.Contains(Spec.Ability->GetClass()))
		{
			AbilitiesToRemove.Add(Spec.Handle);
		}
	}

	for (int32 i = 0; i < AbilitiesToRemove.Num(); i++)
	{
		AbilitySystemComponent->ClearAbility(AbilitiesToRemove[i]);
	}

	AbilitySystemComponent->bCharacterAbilitiesGiven = false;
}

void AASCharacter::AddCharacterAbilities()
{
	// Grant abilities only on the server
	if (GetLocalRole() != ROLE_Authority || !AbilitySystemComponent || AbilitySystemComponent->bCharacterAbilitiesGiven)
	{
		return;
	}

	for (TSubclassOf<UASGameplayAbility>& StartupAbility : CharacterAbilities)
	{
		if (StartupAbility)
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(StartupAbility, 1);
			AbilitySpec.SourceObject = this;
			AbilitySpec.DynamicAbilityTags.AddTag(StartupAbility.GetDefaultObject()->InputTag);
			AbilitySystemComponent->GiveAbility(AbilitySpec);
		}
	}
	AbilitySystemComponent->bCharacterAbilitiesGiven = true;
}

void AASCharacter::AddDeathAbility()
{
	// Granted once per ASC
	if (GetLocalRole() != ROLE_Authority || !AbilitySystemComponent || AbilitySystemComponent->bDeathAbilityGiven || !DeathAbility)
	{
		return;
	}

	FGameplayAbilitySpec AbilitySpec(DeathAbility, 1);
	AbilitySystemComponent->GiveAbility(AbilitySpec);
	AbilitySystemComponent->bDeathAbilityGiven = true;
}

void AASCharacter::InitializeAttributes()
{
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogAS, Warning, TEXT("No AbilitySystemComponent!"));
		return;
	}

	if (!DefaultAttributes)
	{
		UE_LOG(LogAS, Warning, TEXT("No DefaultAttributes effect!"));
		return;
	}

	//can run on server and client
	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	FGameplayEffectSpecHandle EffectSpec = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributes, GetAbilityLevel(), EffectContext);
	if (EffectSpec.IsValid())
	{
		FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*EffectSpec.Data.Get(), AbilitySystemComponent); 
	}
}

void AASCharacter::SetHealth(float Health)
{
	if (CombatAttributes)
	{
		CombatAttributes->SetHealth(Health);
	}
}

void AASCharacter::SetShield(float Health)
{
	if (CombatAttributes)
	{
		CombatAttributes->SetShield(Health);
	}
}

AASWeapon* AASCharacter::GetCurrentWeapon() const
{
	return CurrentWeapon;
}

int32 AASCharacter::GetNumWeapons() const
{
	return Inventory.Num();
}

bool AASCharacter::AddWeaponToInventory(AASWeapon* NewWeapon, bool bEquipWeapon)
{
	// if we already have weapon - add ammo to that weapon
	if (DoesWeaponExistInInventory(NewWeapon))
	{
		if (GetLocalRole() < ROLE_Authority)
		{
			return false;
		}

		NewWeapon->Destroy();

		return false;
	}
	
	if (GetLocalRole() < ROLE_Authority)
	{
		return false;
	}

	Inventory.Add(NewWeapon);
	NewWeapon->SetOwningCharacter(this);
	NewWeapon->AddAbilities();

	if (bEquipWeapon)
	{
		SetActiveWeaponSlotAuthoritative(Inventory.Num() - 1);
	}
	
	return true;
}

bool AASCharacter::RemoveWeaponFromInventory(AASWeapon* WeaponToRemove)
{
	if (DoesWeaponExistInInventory(WeaponToRemove))
	{
		if (WeaponToRemove == CurrentWeapon)
		{
			UnEquipCurrentWeapon();
		}

		Inventory.Remove(WeaponToRemove);
		WeaponToRemove->RemoveAbilities();
		WeaponToRemove->SetOwningCharacter(nullptr);
		WeaponToRemove->Destroy();

		return true;
	}
	
	return false;
}

void AASCharacter::RemoveAllWeaponsFromInventory()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		return;
	}

	UnEquipCurrentWeapon();
	
	for (int32 i = Inventory.Num() - 1; i >= 0; i--)
	{
		AASWeapon* Weapon = Inventory[i];
		RemoveWeaponFromInventory(Weapon);
		
		//TODO set weapon as a pickup
	}
}

void AASCharacter::EquipWeapon(AASWeapon* NewWeapon)
{
	const int32 Slot = Inventory.Find(NewWeapon);
	if (Slot != INDEX_NONE)
	{
		RequestSwitch(Slot);
	}
}

void AASCharacter::NextWeapon()
{
	if (!IsLocallyControlled() || Inventory.Num() < 2)
	{
		return;
	}
	const int32 Current = (PredictedSlot < 0) ? Inventory.Num() - 1 : PredictedSlot;
	const int32 Next = (Current + 1) % Inventory.Num();
	RequestSwitch(Next);
}

void AASCharacter::PreviousWeapon()
{
	if (!IsLocallyControlled() || Inventory.Num() < 2)
	{
		return;
	}
	const int32 Current = (PredictedSlot < 0) ? 0 : PredictedSlot;
	const int32 Previous = (Current - 1 + Inventory.Num()) % Inventory.Num();
	RequestSwitch(Previous);
}

void AASCharacter::RequestSwitch(int32 NewSlot)
{
	if (!IsLocallyControlled() || !Inventory.IsValidIndex(NewSlot) || NewSlot == PredictedSlot)
	{
		return;
	}

	PredictedSlot = NewSlot;
	++LocalSeq;
	ApplyWeaponVisual(NewSlot);
	ServerSetActiveWeapon(NewSlot,LocalSeq);
}

int32 AASCharacter::GetActiveSlotIndex() const
{
	return IsLocallyControlled() ? PredictedSlot : ReplicatedWeaponState.SlotIndex;
}

void AASCharacter::ServerSetActiveWeapon_Implementation(int32 NewSlot, uint8 Seq)
{
	if (!Inventory.IsValidIndex(NewSlot))
	{
		return;
	}
	ReplicatedWeaponState.SlotIndex = NewSlot;
	ReplicatedWeaponState.Seq = Seq;
	ApplyWeaponVisual(NewSlot);
}

bool AASCharacter::ServerSetActiveWeapon_Validate(int32 NewSlot, uint8 Seq)
{
	return true;
}

void AASCharacter::SetActiveWeaponSlotAuthoritative(int32 NewSlot)
{
	if (!HasAuthority() || !Inventory.IsValidIndex(NewSlot))
	{
		return;
	}
	ReplicatedWeaponState.SlotIndex = NewSlot;
	if (IsLocallyControlled())
	{
		PredictedSlot = NewSlot;
	}
	ApplyWeaponVisual(NewSlot);
}

void AASCharacter::OnRep_ReplicatedWeaponState()
{
	if (IsLocallyControlled())
	{
		//Only adopt the server's value once it has caught up to our latest request.
		if (ReplicatedWeaponState.Seq == LocalSeq)
		{
			if (ReplicatedWeaponState.SlotIndex != PredictedSlot)
			{
				PredictedSlot = ReplicatedWeaponState.SlotIndex; // server correction / initial
				ApplyWeaponVisual(PredictedSlot);
			}
		}
	}
	else
	{
		ApplyWeaponVisual(ReplicatedWeaponState.SlotIndex);
	}
}

void AASCharacter::ApplyWeaponVisual(int32 Slot)
{
	if (!Inventory.IsValidIndex(Slot))
	{
		return;
	}
	SetCurrentWeapon(Inventory[Slot], CurrentWeapon);
}

void AASCharacter::SetCurrentWeapon(AASWeapon* NewWeapon, AASWeapon* LastWeapon)
{
	// TODO check(AbilitySystemComponent) or ensure(AbilitySystemComponent) instead of double checking
	if (NewWeapon == LastWeapon)
	{
		return;
	}

	// --- Ammo hot-swap (server-authoritative + owner-predicted; skip simulated proxies) ---
	const bool bOwnsAmmo = AbilitySystemComponent && (HasAuthority() || IsLocallyControlled());

	// SAVE: the attribute currently holds the outgoing weapon's live ammo.
	if (bOwnsAmmo && LastWeapon)
	{
		const float LiveAmmo = AbilitySystemComponent->GetNumericAttribute(UASWeaponAttributeSet::GetAmmoAttribute());
		LastWeapon->SetPersistentAmmo(LiveAmmo);
	}

	// Cancel active weapon abilities
	if (AbilitySystemComponent)
	{
		FGameplayTagContainer With(FASGameplayTags::Ability_Weapon);
		FGameplayTagContainer Without(FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.IsChanging")));
		AbilitySystemComponent->CancelAbilities(&With, &Without);
	}

	UnEquipWeapon(LastWeapon);

	if (NewWeapon)
	{
		if (AbilitySystemComponent)
		{
			// Clear out potential NoWeaponTag
			AbilitySystemComponent->RemoveLooseGameplayTag(CurrentWeaponTag);
		}
		
		CurrentWeapon = NewWeapon;

		// LOAD: push the new weapon's stored ammo into the live attribute.
		if (bOwnsAmmo)
		{
			AbilitySystemComponent->SetNumericAttributeBase(UASWeaponAttributeSet::GetAmmoAttribute(),
				static_cast<float>(NewWeapon->GetPersistentAmmo()));
		}
		
		CurrentWeapon->SetOwningCharacter(this);
		CurrentWeapon->Equip();
		CurrentWeaponTag = CurrentWeapon->WeaponTag;

		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->AddLooseGameplayTag(CurrentWeaponTag);
		}

		AASPlayerController* PC = GetController<AASPlayerController>();
		
		// Animation
		SelectBestLayer(NewWeapon);
		UAnimMontage* Equip1PMontage = CurrentWeapon->GetEquip1PMontage();
		float LockTime = 0.0f;
		if (Equip1PMontage && GetMesh1P() && GetMesh1P()->GetAnimInstance())
		{
			LockTime = GetMesh1P()->GetAnimInstance()->Montage_Play(Equip1PMontage);
		}

		if (bOwnsAmmo)
		{
			const FGameplayTag ChangingTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.IsChanging"));
			if (LockTime > 0.0f)
			{
				AbilitySystemComponent->SetLooseGameplayTagCount(ChangingTag, 1); // block fire
				GetWorldTimerManager().SetTimer(EquipLockTimerHandle, [this,ChangingTag]()
				{
					if (AbilitySystemComponent)
					{
						AbilitySystemComponent->SetLooseGameplayTagCount(ChangingTag,0); // unblock
					}
				}, LockTime, false);
			}
			else
			{
				AbilitySystemComponent->SetLooseGameplayTagCount(ChangingTag,0); // no montage -> no lock
			}
		}

		UAnimMontage* Equip3PMontage = CurrentWeapon->GetEquip3PMontage();
		if (Equip3PMontage && GetMesh())
		{
			GetMesh()->GetAnimInstance()->Montage_Play(Equip3PMontage);
		}
	}
	else
	{
		if (bOwnsAmmo)
		{
			AbilitySystemComponent->SetNumericAttributeBase(UASWeaponAttributeSet::GetAmmoAttribute(),0.f);
			AbilitySystemComponent->SetLooseGameplayTagCount(FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.IsChanging")),0);
		}
		//clear hud, tags etc
		UnEquipCurrentWeapon();
	}
	OnCurrentWeaponChanged.Broadcast();
}

void AASCharacter::UnEquipWeapon(AASWeapon* WeaponToUnEquip)
{
	//TODO this will run into issues when calling UnEquipWeapon explicitly and the WeaponToUnEquip == CurrentWeapon

	if (WeaponToUnEquip)
	{
		WeaponToUnEquip->UnEquip();
	}
}

void AASCharacter::UnEquipCurrentWeapon()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(CurrentWeaponTag);
		CurrentWeaponTag = FASGameplayTags::Weapon_None;
		AbilitySystemComponent->AddLooseGameplayTag(CurrentWeaponTag);
	}

	UnEquipWeapon(CurrentWeapon);
	CurrentWeapon = nullptr;

	AASPlayerController* PC = GetController<AASPlayerController>();
}

void AASCharacter::SelectBestLayer(const AASWeapon* Weapon)
{
	// TODO select unarmed layer if there is no matching
	
	if (!Weapon)
	{
		UE_LOG(LogAS, Warning, TEXT("SelectBestLayer: Weapon is null"));
		return;
	}
	
	if (!Mesh1P || !GetMesh())
	{
		UE_LOG(LogAS, Warning, TEXT("Can't select Anim Layer, Mesh1P: %s or Mesh3P: %s is null"), *GetNameSafe(Mesh1P), *GetNameSafe(GetMesh()));
		return;
	}
	
	const FWeaponAnimLayers* LinkedLayer = WeaponAnimLayers.FindByPredicate(
	[&](const FWeaponAnimLayers& Layer)
	{
		return Layer.WeaponTag == Weapon->WeaponTag;
	});
	if (!LinkedLayer) return;
	
	if (CurrentFirstPersonLayerClass)
	{
		Mesh1P->UnlinkAnimClassLayers(CurrentFirstPersonLayerClass);
	}
	if (LinkedLayer->FPLinkedLayer)
	{
		Mesh1P->LinkAnimClassLayers(LinkedLayer->FPLinkedLayer);
	}
	CurrentFirstPersonLayerClass = LinkedLayer->FPLinkedLayer;

	if (CurrentThirdPersonLayerClass)
	{
		GetMesh()->UnlinkAnimClassLayers(CurrentThirdPersonLayerClass);
	}
	if (LinkedLayer->TPLinkedLayer)
	{
		GetMesh()->LinkAnimClassLayers(LinkedLayer->TPLinkedLayer);
	}
	CurrentThirdPersonLayerClass = LinkedLayer->TPLinkedLayer;
	
}

void AASCharacter::SpawnDefaultInventory()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		return;
	}

	int32 NumWeaponClasses = DefaultInventoryClasses.Num();
	for (int32 i = 0; i < NumWeaponClasses; i++)
	{
		if (!DefaultInventoryClasses[i])
		{
			// An empty item was added to the Array in blueprint
			continue;
		}

		AASWeapon* NewWeapon = GetWorld()->SpawnActorDeferred<AASWeapon>(DefaultInventoryClasses[i],
				FTransform::Identity, this, this, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		NewWeapon->FinishSpawning(FTransform::Identity); // manually call construction after SpawnActorDeferred

		bool bEquipFirstWeapon = i == 0;
		AddWeaponToInventory(NewWeapon, bEquipFirstWeapon);
	}
	OnInventoryChanged.Broadcast();
}

bool AASCharacter::DoesWeaponExistInInventory(AASWeapon* InWeapon)
{
	for (AASWeapon* Weapon : Inventory)
	{
		if (Weapon && InWeapon && Weapon->GetClass() == InWeapon->GetClass())
		{
			return true;
		}
	}
		
	return false;
}

void AASCharacter::OnRep_Inventory()
{
	// The active slot may have replicated before the inventory/weapon actors. Re-apply now.
	const int32 Slot = IsLocallyControlled() ? PredictedSlot : ReplicatedWeaponState.SlotIndex;
	ApplyWeaponVisual(Slot);
	OnInventoryChanged.Broadcast(); 
}

// Server-only
void AASCharacter::StartDeath(const FVector& ImpulseDir, const FVector& ImpulseLocation, FName ImpulseBone)
{
	if (DeathState.bIsDead)
	{
		return;
	}
	
	DeathState.bIsDead = true;
	DeathState.ImpulseDir = ImpulseDir;
	DeathState.ImpulseLocation = ImpulseLocation;
	DeathState.ImpulseBone = ImpulseBone;

	RemoveAllWeaponsFromInventory();
	RemoveCharacterAbilities();
	
	if (AController* C = GetController())
	{
		C->SetIgnoreMoveInput(true);
	}

	// Server-side ragdoll so simulated proxies and the authoritative sim stay in sync.
	SetRagdollPhysics();
}

// Server and on all clients
void AASCharacter::SetRagdollPhysics()
{
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	if (Mesh1P)
	{
		Mesh1P->SetVisibility(false);
	}

	USkeletalMeshComponent* Mesh3P = GetMesh();
	if (Mesh3P && Mesh3P->GetPhysicsAsset())
	{
		Mesh3P->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh3P->SetCollisionProfileName(FName(TEXT("Ragdoll")));
		Mesh3P->SetAllBodiesSimulatePhysics(true);
		Mesh3P->SetSimulatePhysics(true);
		Mesh3P->WakeAllRigidBodies();
		Mesh3P->bBlendPhysics = true;

		ApplyDeathImpulse();
	}
	else if (Mesh3P)
	{
		// No physics asset to ragdoll with: just hide the body as a fallback.
		Mesh3P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh3P->SetVisibility(false);
	}
}

void AASCharacter::ApplyDeathImpulse()
{
	USkeletalMeshComponent* Mesh3P = GetMesh();
	if (!Mesh3P || !Mesh3P->IsSimulatingPhysics() || DeathState.ImpulseDir.IsNearlyZero())
	{
		return;
	}

	const FVector Impulse = DeathState.ImpulseDir * DeathImpulseStrength;
	if (DeathState.ImpulseBone != NAME_None)
	{
		Mesh3P->AddImpulseAtLocation(Impulse, DeathState.ImpulseLocation, DeathState.ImpulseBone);
	}
	else
	{
		Mesh3P->AddImpulse(Impulse, NAME_None, false);
	}
}

void AASCharacter::OnRep_DeathState()
{
	if (DeathState.bIsDead)
	{
		SetRagdollPhysics();
	}
}

USkeletalMeshComponent* AASCharacter::GetMesh1P() const
{
	return Mesh1P;
}

FName AASCharacter::GetWeapon1PAttachPoint() const
{
	return WeaponAttachSocket1P;
}
FName AASCharacter::GetWeapon3PAttachPoint() const
{
	return WeaponAttachSocket3P;
}

AASWeapon* AASCharacter::FindWeaponByTag(FGameplayTag Tag) const
{
	for (AASWeapon* Weapon : Inventory)
	{
		if (Weapon && Weapon->WeaponTag.MatchesTagExact(Tag))
		{
			return Weapon;
		}
	}
	
	return nullptr;
}

TArray<FName> AASCharacter::Get1PSockets() const
{
	TArray<FName> Out;
	if (const USkeletalMeshComponent* mesh1P = GetMesh1P())
	{
		Out = mesh1P->GetAllSocketNames(); // sockets on the 1P mesh
	}
	return Out;
}

TArray<FName> AASCharacter::Get3PSockets() const
{
	TArray<FName> Out;
	if (const USkeletalMeshComponent* mesh3P = GetMesh())
	{
		Out = mesh3P->GetAllSocketNames(); // sockets on the 3P mesh
	}
	return Out;
}

void AASCharacter::Move(const FInputActionValue& Value)
{
	if (!FMath::IsNearlyZero(Value.GetMagnitude()))
	{
		// add movement in that direction
		FVector MovementVector = Value.Get<FVector>();
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}

}

void AASCharacter::LookUp(const FInputActionValue& Value)//bool bIsPure, float Rate)
{
	float Rate = Value.GetMagnitude();//Value.Get<FVector>().GetSafeNormal().Size(); //Value.Get<FVector>().Size();
	/*if (!bIsPure)
	{*/
	//	Rate = Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds();
	//}

	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate);
}


void AASCharacter::Turn(const FInputActionValue& Value)
{
	float Rate = Value.GetMagnitude();
	//if (!bIsPure)
	//{
	//	Rate = Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds();
	//}
	AddControllerYawInput(Rate);
}

