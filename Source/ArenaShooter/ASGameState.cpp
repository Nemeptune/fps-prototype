// Fill out your copyright notice in the Description page of Project Settings.


#include "ASGameState.h"
#include "Net/UnrealNetwork.h"

void AASGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AASGameState, RemainingTime);
	DOREPLIFETIME(AASGameState, bTimerPaused);
	DOREPLIFETIME(AASGameState, WinnerPS);
	DOREPLIFETIME(AASGameState, bMatchEnded);
}

void AASGameState::MulticastOnPlayerJoined_Implementation()
{
	OnPlayerJoined.Broadcast();
}

void AASGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
	OnPlayerAdded.Broadcast(PlayerState);
}

void AASGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);
	OnPlayerRemoved.Broadcast(PlayerState);
}

void AASGameState::SetMatchResult(APlayerState* InWinner)
{
	WinnerPS = InWinner;
	bMatchEnded = true;
	OnMatchEnded.Broadcast(); 
	ForceNetUpdate();
}

APlayerState* AASGameState::GetWinner() const
{
	return WinnerPS;
}

bool AASGameState::HasMatchEnded() const
{
	return bMatchEnded;
}

void AASGameState::OnRep_MatchEnded()
{
	OnMatchEnded.Broadcast();
}
