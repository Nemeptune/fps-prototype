


#include "ASWeapon.h"

#include "Net/UnrealNetwork.h"
#include "ASCharacter.h"
#include "ASLogChannels.h"
#include "AbilitySystem/ASAbilitySystemComponent.h"
#include "AbilitySystem/ASWeaponAttributeSet.h"


AASWeapon::AASWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	
	USceneComponent* WeaponRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponRoot"));
	SetRootComponent(WeaponRoot);
	
	WeaponMesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh1P"));
	WeaponMesh1P->SetupAttachment(WeaponRoot);
	WeaponMesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh1P->CastShadow = false;
	WeaponMesh1P->SetOnlyOwnerSee(true);

	WeaponMesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh3P"));
	WeaponMesh3P->SetupAttachment(WeaponRoot);
	WeaponMesh3P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh3P->CastShadow = true;
	WeaponMesh3P->SetOwnerNoSee(true);

	bReplicates = true;
}

void AASWeapon::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		PersistentAmmo = StartingAmmo;
	}
}

void AASWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (WeaponMesh1P) { WeaponMesh1P->SetVisibility(false, true); }
	if (WeaponMesh3P) { WeaponMesh3P->SetVisibility(false, true); }
}

void AASWeapon::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	RemoveAbilities();
	
	Super::EndPlay(EndPlayReason);
}

void AASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AASWeapon, PersistentAmmo, COND_OwnerOnly);
}

void AASWeapon::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);
	//TODO write DOREPLIFETIME_ACTIVE_OVERRIDE for ClipAmmo managment
}

void AASWeapon::OnRep_PersistentAmmo()
{
	if (OwningCharacter)
	{
		OwningCharacter->OnInventoryChanged.Broadcast();
	}
}

void AASWeapon::Fire_Implementation(const TArray<FVector>& impactPositions, const TArray<FVector>& impactNormals,
                                    const TArray<TEnumAsByte<EPhysicalSurface>>& impactSurfaceTypes)
{
	
}

void AASWeapon::SetOwningCharacter(AASCharacter* InOwningCharacter)
{
	OwningCharacter = InOwningCharacter;
	if (OwningCharacter)
	{
		SetOwner(OwningCharacter);
		AttachToComponent(OwningCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

		if (OwningCharacter->GetCurrentWeapon() != this)
		{
			WeaponMesh3P->CastShadow = false;
			WeaponMesh3P->SetVisibility(false, true);
		}
	}else
	{
		UE_LOG(LogTemp, Error, TEXT("WEAPON DETACHED: %s  Role=%d"), *GetName(), (int32)GetLocalRole());
		SetOwner(nullptr);
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
}

void AASWeapon::Equip()
{
	if (!OwningCharacter)
	{
		return;
	}
	
	if (WeaponMesh1P)
	{
		WeaponMesh1P->AttachToComponent(OwningCharacter->GetMesh1P(), FAttachmentTransformRules::SnapToTargetIncludingScale, OwningCharacter->GetWeapon1PAttachPoint());
		WeaponMesh1P->SetVisibility(true, true);
	}
	if (WeaponMesh3P)
	{
		WeaponMesh3P->AttachToComponent(OwningCharacter->GetMesh(),  FAttachmentTransformRules::SnapToTargetIncludingScale, OwningCharacter->GetWeapon3PAttachPoint());
		WeaponMesh3P->bCastHiddenShadow = true;
		WeaponMesh3P->SetVisibility(true, true);
	}
}

void AASWeapon::UnEquip()
{
	if (!OwningCharacter)
	{
		return;
	}

	if (WeaponMesh1P)
	{
		WeaponMesh1P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		WeaponMesh1P->SetVisibility(false, true);
	}
	if (WeaponMesh3P)
	{
		WeaponMesh3P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		WeaponMesh3P->bCastHiddenShadow = false;
		WeaponMesh3P->SetVisibility(false, true);
	}
}

void AASWeapon::AddAbilities()
{
	if (!IsValid(OwningCharacter) || !OwningCharacter->GetAbilitySystemComponent())
	{
		UE_LOG(LogAS, Error, TEXT("OwningCharacter or asc not valid"));
		return;
	}
	UASAbilitySystemComponent* ASC = Cast<UASAbilitySystemComponent>(OwningCharacter->GetAbilitySystemComponent());

	if (!ASC)
	{
		UE_LOG(LogAS, Error, TEXT("%s %s Role: NAN ASC is null"), *FString(__FUNCTION__), *GetName());
		return;
	}

	// Grant abilities, but only on the server	
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}
	UE_LOG(LogAS, Error, TEXT("AASWeapon::AddAbilities()"));
	AbilitySet->GiveToAbilitySystem(ASC, &GrantedHandles, this);
}

void AASWeapon::RemoveAbilities()
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if (!IsValid(OwningCharacter) || !OwningCharacter->GetAbilitySystemComponent())
	{
		return;
	}

	UASAbilitySystemComponent* ASC = Cast<UASAbilitySystemComponent>(OwningCharacter->GetAbilitySystemComponent());

	GrantedHandles.TakeFromAbilitySystem(ASC);
}

int32 AASWeapon::GetPersistentAmmo() const
{
	return PersistentAmmo;
}

void AASWeapon::SetPersistentAmmo(int32 NewValue)
{
	PersistentAmmo = FMath::Max(0, NewValue);
}

void AASWeapon::AddAmmo(int32 Value)
{
	if (!HasAuthority() || Value <= 0)
	{
		return;
	}

	UAbilitySystemComponent* ASC = OwningCharacter ? OwningCharacter->GetAbilitySystemComponent() : nullptr;
	if (ASC && OwningCharacter->GetCurrentWeapon() == this)
	{
		ASC->ApplyModToAttribute(UASWeaponAttributeSet::GetAmmoAttribute(), EGameplayModOp::Additive, static_cast<float>(Value));
	}
	else
	{
		SetPersistentAmmo(PersistentAmmo + Value);
		if (OwningCharacter)
		{
			OwningCharacter->OnInventoryChanged.Broadcast();
		}
	}
}

FTransform AASWeapon::GetMuzzleTransform(FName Socket) const
{
	if (OwningCharacter)
	if (WeaponMesh1P && WeaponMesh1P->DoesSocketExist(Socket))
	{
		return WeaponMesh1P->GetSocketTransform(Socket);
	}

	return GetActorTransform(); // may be confusing if not working properly
}

USkeletalMeshComponent* AASWeapon::GetWeaponMesh1P() const
{
	return WeaponMesh1P;
}

USkeletalMeshComponent* AASWeapon::GetWeaponMesh3P() const
{
	return WeaponMesh3P;
}

UAnimMontage* AASWeapon::GetEquip1PMontage() const
{
	return Equip1PMontage;
}

UAnimMontage* AASWeapon::GetEquip3PMontage() const
{
	return Equip3PMontage;
}

TSubclassOf<UAnimInstance> AASWeapon::GetFirstPersonAnimLayer() const
{
	return FirstPersonAnimLayer;
}

TSubclassOf<UAnimInstance> AASWeapon::GetThirdPersonAnimLayer() const
{
	return ThirdPersonAnimLayer;
}

const FSlateBrush& AASWeapon::GetIcon() const
{
	return Icon;
}
