// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ASUIManager.h"

#include "ASCharacter.h"
#include "ASCombatAttributeSet.h"
#include "ASGameState.h"
#include "ASHUDViewModel.h"
#include "ASHUDWidget.h"
#include "ASLogChannels.h"
#include "ASPlayerController.h"
#include "ASPlayerState.h"
#include "ASResultWidget.h"
#include "ASWeapon.h"
#include "ASWeaponSlotViewModel.h"
#include "MVVMSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "AbilitySystem/ASAbilitySystemComponent.h"
#include "AbilitySystem/ASWeaponAttributeSet.h"
#include "View/MVVMView.h"

// TODO too much silent returns in this class, if smth goes wrong there are no clue what caused it

void UASUIManager::Deinitialize()
{
	ResetState();
	Super::Deinitialize();
}

void UASUIManager::OnLocalPlayerReady(AASPlayerController* PC, TSubclassOf<UUserWidget> HUDClass)
{
	if (!PC || !PC->IsLocalController()) return;
	AASPlayerState* PS = PC->GetPlayerState<AASPlayerState>();
	if (!PS) return; // PC will call again on OnRep_PlayerState

	if (LocalPS != PS) ResetState(); // new context (map travel)
	LocalPS = PS;
	ResultWidgetClass = PC->GetResultWidgetClass();

	EnsureHUD(PC, HUDClass);
	BindLocalPlayerState();
	BindWorldState();
	BindPawn(PC);
}

void UASUIManager::EnsureHUD(AASPlayerController* PC, TSubclassOf<UUserWidget> HUDClass)
{
	if (HUDWidget || !HUDClass || !PC) return;

	HUDVM = NewObject<UASHUDViewModel>(this);
	HUDWidget = CreateWidget<UASHUDWidget>(PC, HUDClass);
	if (!HUDWidget) return;
	HUDWidget->AddToViewport();
	AssignViewModel();
	InitHotBar();
}

void UASUIManager::AssignViewModel()
{
	if (!HUDWidget || !HUDVM) return;
	if (UMVVMSubsystem* Sub = GEngine->GetEngineSubsystem<UMVVMSubsystem>())
	{
		if (UMVVMView* View = Sub->GetViewFromUserWidget(HUDWidget))
		{
			View->SetViewModelByClass(HUDVM);
		}
	}
}

void UASUIManager::BindLocalPlayerState()
{
	if (bLocalBound || !LocalPS || !HUDVM) return;
	bLocalBound = true;

	HUDVM->SetLocalName(FText::FromString(LocalPS->GetPlayerName()));
	HUDVM->SetLocalKills(LocalPS->GetKills());
	LocalPS->OnKillsChanged.AddDynamic(this, &UASUIManager::HandleLocalKills);
	LocalPS->OnNameChanged.AddDynamic(this, &UASUIManager::HandleLocalNameChanged);

	if (UASAbilitySystemComponent* ASC = LocalPS->GetASAbilityComponent())
	{
		BoundASC = ASC;
		ASC->GetGameplayAttributeValueChangeDelegate(UASCombatAttributeSet::GetHealthAttribute()).AddUObject(this, &UASUIManager::HandleHealthChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(UASCombatAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &UASUIManager::HandleMaxHealthChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(UASCombatAttributeSet::GetShieldAttribute()).AddUObject(this, &UASUIManager::HandleShieldChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(UASCombatAttributeSet::GetMaxShieldAttribute()).AddUObject(this, &UASUIManager::HandleMaxShieldChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(UASWeaponAttributeSet::GetAmmoAttribute()).AddUObject(this, &UASUIManager::HandleAmmoChanged);

		// initial push
		HUDVM->SetMaxHealth(ASC->GetNumericAttribute(UASCombatAttributeSet::GetMaxHealthAttribute()));
		HUDVM->SetHealth(ASC->GetNumericAttribute(UASCombatAttributeSet::GetHealthAttribute()));
		HUDVM->SetMaxShield(ASC->GetNumericAttribute(UASCombatAttributeSet::GetMaxShieldAttribute()));
		HUDVM->SetShield(ASC->GetNumericAttribute(UASCombatAttributeSet::GetShieldAttribute()));
	}
}

void UASUIManager::BindWorldState()
{
	if (bWorldBound) return;
	AASGameState* GS = GetWorld() ? GetWorld()->GetGameState<AASGameState>() : nullptr;
	if (!GS)
	{
		if (UWorld* World = GetWorld()) // GameState not replicated yet — retry next tick
		{
			World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UASUIManager::BindWorldState));
			return;
		}
		return;
	}
	bWorldBound = true;
	BoundGS = GS;

	GS->OnPlayerAdded.AddUObject(this, &UASUIManager::HandlePlayerAdded);
	GS->OnPlayerRemoved.AddUObject(this, &UASUIManager::HandlePlayerRemoved);
	GS->OnMatchEnded.AddUObject(this, &UASUIManager::HandleMatchEnded);
	for (APlayerState* PS : GS->PlayerArray)
	{
		HandlePlayerAdded(PS);
	}

	GetWorld()->GetTimerManager().SetTimer(Timer, this, &UASUIManager::UpdateTimer, 1.f, true);
	UpdateTimer();
}

void UASUIManager::BindPawn(class AASPlayerController* PC)
{
	if (!PC) return;
	
	if (!bPawnBound)
	{
		PC->OnPossessedPawnChanged.AddDynamic(this, &UASUIManager::HandlePawnChanged);
		bPawnBound = true;
	}
	if (APawn* Pawn = PC->GetPawn())
	{
		HandlePawnChanged(nullptr, Pawn);
	}
	else if (!LocalCharacter && GetWorld())
	{
		TWeakObjectPtr<AASPlayerController> WeakPC(PC);
		GetWorld()->GetTimerManager().SetTimerForNextTick([this, WeakPC]()
		{
			if (WeakPC.IsValid())
			{
				BindPawn(WeakPC.Get());
			}
		});
	}
}

void UASUIManager::HandlePlayerAdded(APlayerState* PlayerState)
{
	if (!PlayerState || PlayerState == LocalPS || OpponentPS) return;
	AASPlayerState* OppPS = Cast<AASPlayerState>(PlayerState);
	if (!OppPS) return;

	OpponentPS = OppPS;
	if (HUDVM)
	{
		HUDVM->SetOpponentName(FText::FromString(OppPS->GetPlayerName()));
		HUDVM->SetOpponentKills(OppPS->GetKills());
	}
	OppPS->OnKillsChanged.AddDynamic(this, &UASUIManager::HandleOpponentKills);
	OppPS->OnNameChanged.AddDynamic(this, &UASUIManager::HandleOpponentNameChanged);
}

void UASUIManager::HandlePlayerRemoved(APlayerState* PlayerState)
{
	if (PlayerState != OpponentPS || !OpponentPS) return;
	OpponentPS->OnKillsChanged.RemoveDynamic(this, &UASUIManager::HandleOpponentKills);
	OpponentPS->OnNameChanged.RemoveDynamic(this, &UASUIManager::HandleOpponentNameChanged);
	OpponentPS = nullptr;
	if (HUDVM)
	{
		HUDVM->SetOpponentName(FText::GetEmpty());
		HUDVM->SetOpponentKills(0); // TODO change this when reconnect support added
	}
}

void UASUIManager::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	if (HUDVM)
	{
		HUDVM->SetHealth(Data.NewValue);
	}
}

void UASUIManager::HandleMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	if (HUDVM)
	{
		HUDVM->SetMaxHealth(Data.NewValue);
	}
}

void UASUIManager::HandleShieldChanged(const FOnAttributeChangeData& Data)
{
	if (HUDVM)
	{
		HUDVM->SetShield(Data.NewValue);
	}
}

void UASUIManager::HandleMaxShieldChanged(const FOnAttributeChangeData& Data)
{
	if (HUDVM)
	{
		HUDVM->SetMaxShield(Data.NewValue);
	}
}

void UASUIManager::HandleAmmoChanged(const FOnAttributeChangeData& Data)
{
	const int32 Value = FMath::RoundToInt(Data.NewValue);

	const int32 Slot = LocalCharacter ? LocalCharacter->GetActiveSlotIndex() : INDEX_NONE;
	if (SlotsVM.IsValidIndex(Slot))
	{
		SlotsVM[Slot]->SetAmmo(Value);
	}
}

void UASUIManager::HandleCurrentWeaponChanged()
{
	if (!LocalCharacter) return;
	
	const int32 NewSlot = LocalCharacter->GetActiveSlotIndex();
	if (SlotsVM.IsValidIndex(ActiveSlot))
	{
		SlotsVM[ActiveSlot]->SetIsActive(false);
	}
	ActiveSlot = NewSlot;
	if (SlotsVM.IsValidIndex(ActiveSlot))
	{
		SlotsVM[ActiveSlot]->SetIsActive(true);
	}
}

void UASUIManager::HandleMatchEnded()
{
	AASGameState* GS = BoundGS.Get();
	APlayerController* PC = GetLocalPlayer() ? GetLocalPlayer()->GetPlayerController(GetWorld()) : nullptr;
	if (!GS || !PC || !ResultWidgetClass || ResultWidget)
	{
		return;
	}

	const bool bDraw = (GS->GetWinner() == nullptr);
	const bool bWon  = !bDraw && (GS->GetWinner() == LocalPS);

	ResultWidget = CreateWidget<UASResultWidget>(PC, ResultWidgetClass);
	if (!ResultWidget)
	{
		return;
	}
	ResultWidget->InitResult(bWon, bDraw);
	ResultWidget->AddToViewport(10);

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(ResultWidget->TakeWidget());
	PC->SetInputMode(InputMode);
	PC->SetShowMouseCursor(true);
}

void UASUIManager::HandleLocalNameChanged()
{
	if (HUDVM && LocalPS)
	{
		HUDVM->SetLocalName(FText::FromString(LocalPS->GetPlayerName()));
	}
}

void UASUIManager::HandleOpponentNameChanged()
{
	if (HUDVM && OpponentPS)
	{
		HUDVM->SetOpponentName(FText::FromString(OpponentPS->GetPlayerName()));
	}
}

void UASUIManager::HandlePawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	AASCharacter* NewCharacter = Cast<AASCharacter>(NewPawn);

	if (NewCharacter == LocalCharacter) return;

	if (LocalCharacter)
	{
		LocalCharacter->OnCurrentWeaponChanged.RemoveAll(this);
	}
	
	LocalCharacter = NewCharacter;
	
	if (LocalCharacter)
	{
		LocalCharacter->OnCurrentWeaponChanged.AddUObject(this, &UASUIManager::HandleCurrentWeaponChanged);
		LocalCharacter->OnInventoryChanged.AddUObject(this, &UASUIManager::RefreshHotbar);
		RefreshHotbar();
	}
}

void UASUIManager::HandleLocalKills(int32 NewKills)
{
	if (HUDVM)
	{
		HUDVM->SetLocalKills(NewKills);
	}
}

void UASUIManager::HandleOpponentKills(int32 NewKills)
{
	if (HUDVM)
	{
		HUDVM->SetOpponentKills(NewKills);
	}
}

void UASUIManager::InitHotBar()
{
	if (!HUDWidget || SlotsVM.Num() > 0) return;

	SlotsVM.Reset();
	TArray<UASWeaponSlotViewModel*> Raw;
	Raw.Reserve(HUDWidget->NumWeaponSlots);                 // NumWeaponSlots = 2 (match the placed widgets)
	for (int32 i = 0; i < HUDWidget->NumWeaponSlots; ++i)
	{
		UASWeaponSlotViewModel* VM = NewObject<UASWeaponSlotViewModel>(this);
		SlotsVM.Add(VM);
		Raw.Add(VM);
	}
	HUDWidget->InitWeaponSlots(Raw);               // assign to Slot0/Slot1 — always fires
}

void UASUIManager::RefreshHotbar()
{
	if (!LocalCharacter || SlotsVM.Num() == 0) return;

	const TArray<TObjectPtr<AASWeapon>>& Inventory = LocalCharacter->GetInventory();
	ActiveSlot = LocalCharacter->GetActiveSlotIndex();

	for (int32 i = 0; i < SlotsVM.Num(); ++i)
	{
		const bool bActive = (i == ActiveSlot);
		const int32 Ammo = Inventory.IsValidIndex(i)
			? (bActive ? GetCurrentAmmo() : (Inventory[i] ? Inventory[i]->GetPersistentAmmo() : 0))
			: 0;
		AASWeapon* W = Inventory.IsValidIndex(i) ? Inventory[i].Get() : nullptr;
		SlotsVM[i]->SetIcon(W ? W->GetIcon() : FSlateBrush());
		SlotsVM[i]->SetIsActive(bActive);
		SlotsVM[i]->SetAmmo(Ammo);
	}
}

void UASUIManager::UpdateTimer()
{
	AASGameState* GS = GetWorld() ? GetWorld()->GetGameState<AASGameState>() : nullptr;
	if (!GS || !HUDVM) return;
	const int32 RemainingTime = FMath::Max(0, GS->RemainingTime);
	HUDVM->SetMatchTimeText(FText::FromString(FString::Printf(TEXT("%02d:%02d"), RemainingTime / 60, RemainingTime % 60)));
}

int32 UASUIManager::GetCurrentAmmo() const
{
	if (LocalPS)
	{
		if (UASAbilitySystemComponent* ASC = LocalPS->GetASAbilityComponent())
		{
			return FMath::RoundToInt(ASC->GetNumericAttribute(UASWeaponAttributeSet::GetAmmoAttribute()));
		}
	}
	return 0;
}

void UASUIManager::ResetState()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(Timer);
	}
	if (LocalPS)
	{
		LocalPS->OnKillsChanged.RemoveDynamic(this, &UASUIManager::HandleLocalKills);
		LocalPS->OnNameChanged.RemoveDynamic(this, &UASUIManager::HandleLocalNameChanged);
	}
	if (OpponentPS)
	{
		OpponentPS->OnKillsChanged.RemoveDynamic(this, &UASUIManager::HandleOpponentKills);
		OpponentPS->OnNameChanged.RemoveDynamic(this, &UASUIManager::HandleOpponentNameChanged);
	}
	if (HUDWidget)
	{
		HUDWidget->RemoveFromParent();
	}
	if (LocalCharacter)
	{
		LocalCharacter->OnCurrentWeaponChanged.RemoveAll(this);
		LocalCharacter->OnInventoryChanged.RemoveAll(this);
	}
	if (UASAbilitySystemComponent* ASC = BoundASC.Get())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UASCombatAttributeSet::GetHealthAttribute()).RemoveAll(this);
		ASC->GetGameplayAttributeValueChangeDelegate(UASCombatAttributeSet::GetMaxHealthAttribute()).RemoveAll(this);
		ASC->GetGameplayAttributeValueChangeDelegate(UASCombatAttributeSet::GetShieldAttribute()).RemoveAll(this);
		ASC->GetGameplayAttributeValueChangeDelegate(UASCombatAttributeSet::GetMaxShieldAttribute()).RemoveAll(this);
		ASC->GetGameplayAttributeValueChangeDelegate(UASWeaponAttributeSet::GetAmmoAttribute()).RemoveAll(this);
	}
	BoundASC = nullptr;

	if (AASGameState* GS = BoundGS.Get())
	{
		GS->OnPlayerAdded.RemoveAll(this);
		GS->OnPlayerRemoved.RemoveAll(this);
		GS->OnMatchEnded.RemoveAll(this);
	}
	if (ResultWidget)
	{
		ResultWidget->RemoveFromParent();
		ResultWidget = nullptr;
	}
	BoundGS = nullptr;
	HUDWidget = nullptr;
	HUDVM = nullptr;
	LocalPS = nullptr;
	OpponentPS = nullptr;
	bLocalBound = false;
	bWorldBound = false;
	LocalCharacter = nullptr;
	bPawnBound = false;
	SlotsVM.Reset();
	ActiveSlot = INDEX_NONE;
}
