// Fill out your copyright notice in the Description page of Project Settings.


#include "ASCombatAttributeSet.h"

#include "ASCharacter.h"
#include "ASGameplayTags.h"
#include "ASLogChannels.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "Abilities/GameplayAbilityTypes.h"

UASCombatAttributeSet::UASCombatAttributeSet()
{
	ShieldAbsorptionPercent = 0.6;
}

void UASCombatAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UASCombatAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UASCombatAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UASCombatAttributeSet, Shield, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UASCombatAttributeSet, MaxShield, COND_OwnerOnly, REPNOTIFY_Always);
}

float UASCombatAttributeSet::ASGetHealth() const
{
	return GetHealth();
}

void UASCombatAttributeSet::ASSetHealth(float NewValue)
{
	SetHealth(NewValue);
}

void UASCombatAttributeSet::ASInitHealth(float NewValue)
{
	InitHealth(NewValue);
}

void UASCombatAttributeSet::ASInitMaxHealth(float NewValue)
{
	InitMaxHealth(NewValue);
}

void UASCombatAttributeSet::ASInitShield(float NewValue)
{
	InitShield(NewValue);
}

void UASCombatAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UASCombatAttributeSet, Health, OldValue);
}

void UASCombatAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UASCombatAttributeSet, MaxHealth, OldValue);
}

void UASCombatAttributeSet::OnRep_Shield(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UASCombatAttributeSet, Shield, OldValue);
}

void UASCombatAttributeSet::OnRep_MaxShield(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UASCombatAttributeSet, MaxShield, OldValue);
}

//called on the server
bool UASCombatAttributeSet::PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data)
{
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		if (GetShield() > 0)
		{
			float CapturedDamage = Data.EvaluatedData.Magnitude;
			Data.EvaluatedData.Magnitude *= ShieldAbsorptionPercent;
			SetShield(GetShield() - (CapturedDamage - Data.EvaluatedData.Magnitude));
		}
	}
	return true;
}

void UASCombatAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	AActor* TargetActor = nullptr;
	AASCharacter* TargetCharacter = nullptr;
	
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		TargetCharacter = Cast<AASCharacter>(TargetActor);
	}
	
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		bool WasAlive = true;
		if (TargetCharacter)
		{
			WasAlive = TargetCharacter->IsAlive();			
		}
		
		SetHealth(FMath::Clamp(GetHealth() - Damage.GetCurrentValue(), 0.0f, GetMaxHealth()));
		Damage = 0.0f;

		if (WasAlive && TargetCharacter && !TargetCharacter->IsAlive())
		{
			const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
			AActor* Killer = EffectContext.GetEffectCauser();
			AActor* Victim = GetOwningActor();

			// Hand off to the death ability
			if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
			{
				FGameplayEventData Payload;
				Payload.EventTag = FASGameplayTags::Event_Death;
				Payload.Instigator = Killer;
				Payload.Target = Victim;
				Payload.ContextHandle = EffectContext;
				ASC->HandleGameplayEvent(FASGameplayTags::Event_Death, &Payload);
			}
		}
	}
	else if (Data.EvaluatedData.Attribute == GetHealingAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth() + Healing.GetCurrentValue(), 0.0f, GetMaxHealth()));
		Healing = 0.0f;
	}
	else if (Data.EvaluatedData.Attribute == GetShieldAttribute())
	{
		SetShield(FMath::Clamp(GetShield(), 0.0f, GetMaxShield()));
	}
}
