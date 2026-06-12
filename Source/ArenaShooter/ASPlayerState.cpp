// Fill out your copyright notice in the Description page of Project Settings.


#include "ASPlayerState.h"
#include "AbilitySystem/ASAbilitySystemComponent.h"
#include "ASCombatAttributeSet.h"
#include "AbilitySystem/ASWeaponAttributeSet.h"
#include "Net/UnrealNetwork.h"

AASPlayerState::AASPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UASAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	CombatAttributes = CreateDefaultSubobject<UASCombatAttributeSet>(TEXT("CombatAttributes"));
	WeaponAttributes = CreateDefaultSubobject<UASWeaponAttributeSet>(TEXT("WeaponAttributes"));

	// AbilitySystemComponent needs to be updated at a high frequency.
	NetUpdateFrequency = 100.0f;
}

void AASPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AASPlayerState, Kills);
	DOREPLIFETIME(AASPlayerState, Deaths);
}

UAbilitySystemComponent* AASPlayerState::GetAbilitySystemComponent() const
{
	return GetASAbilityComponent();
}

UASCombatAttributeSet* AASPlayerState::GetCombatAttributeSet() const
{
	return CombatAttributes;
}

UASWeaponAttributeSet* AASPlayerState::GetWeaponAttributeSet() const
{
	return WeaponAttributes;
}

float AASPlayerState::GetHealth() const
{
	return CombatAttributes->GetHealth();
}

float AASPlayerState::GetMaxHealth() const
{
	return CombatAttributes->GetMaxHealth();
}

float AASPlayerState::GetShield() const
{
	return CombatAttributes->GetShield();
}

float AASPlayerState::GetMaxShield() const
{
	return CombatAttributes->GetMaxShield();
}

int32 AASPlayerState::GetKills() const
{
	return Kills;
}

int32 AASPlayerState::GetDeaths() const
{
	return Deaths;
}

void AASPlayerState::OnRep_Kills()
{
	OnKillsChanged.Broadcast(Kills);
}

void AASPlayerState::OnRep_Deaths()
{
	OnDeathsChanged.Broadcast(Deaths);
}

void AASPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();
	OnNameChanged.Broadcast();
}

void AASPlayerState::AddKill()
{
	Kills++;
	OnKillsChanged.Broadcast(Kills);
}

void AASPlayerState::AddDeath()
{
	Deaths++;
	OnDeathsChanged.Broadcast(Deaths);
}
