#include "ASPlayerController.h"
#include "ASPlayerState.h"
#include "AbilitySystem/ASAbilitySystemComponent.h"
#include "UI/ASHUDWidget.h"
#include "Input/ASInputComponent.h"
#include "UI/ASUIManager.h"

void AASPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		CreateHUD();
	}
}

void AASPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	if (!IsLocalController()) return;
	
	if (UASInputComponent* IC = Cast<UASInputComponent>(InputComponent))
	{
		IC->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputPressed, &ThisClass::AbilityInputReleased);
	}
}

void AASPlayerController::SetAbilitySystemComponent(UASAbilitySystemComponent* ASC)
{
	AbilitySystemComponent = ASC;
}

void AASPlayerController::CreateHUD()
{
	if (!IsLocalController()) return;
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UASUIManager* UI = LP->GetSubsystem<UASUIManager>())
		{
			UI->OnLocalPlayerReady(this, UIHUDWidgetClass);
		}
	}
}

UASHUDWidget* AASPlayerController::GetHUD()
{
	return UIHUDWidget;
}

TSubclassOf<UASResultWidget> AASPlayerController::GetResultWidgetClass() const
{
	return ResultWidgetClass;
}

void AASPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	AASPlayerState* PS = GetPlayerState<AASPlayerState>();
	if (PS)
	{
		// Init ASC with PS (Owner) and our new Pawn (AvatarActor)
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, InPawn);
	}
}

void AASPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	// For edge cases where the PlayerState is replicated before the Character is possessed.
	CreateHUD();
}

void AASPlayerController::AbilityInputPressed(FGameplayTag InputTag)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AbilityInputPressed(InputTag);
	}
}

void AASPlayerController::AbilityInputReleased(FGameplayTag InputTag)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AbilityInputReleased(InputTag);
	}
}

