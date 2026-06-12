// Fill out your copyright notice in the Description page of Project Settings.


#include "ASGameplayAbility_RangedWeapon.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "ASLogChannels.h"
#include "AbilitySystem/ASAbilitySystemComponent.h"
#include "AbilitySystem/ASWeaponAttributeSet.h"


UASGameplayAbility_RangedWeapon::UASGameplayAbility_RangedWeapon()
{
	bSourceObjectMustEqualCurrentEquipToActivate = true;
}

bool UASGameplayAbility_RangedWeapon::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	bool bResult =  Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
	if (bResult)
	{
		//Get weapon instance
	}
	
	return bResult;
}

void UASGameplayAbility_RangedWeapon::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilityComponent);

	OnTargetDataReadyCallbackDelegateHandle = MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &ThisClass::OnTargetDataReadyCallback);

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UASGameplayAbility_RangedWeapon::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsEndAbilityValid(Handle, ActorInfo))
	{
		if (ScopeLockCount > 0)
		{
			WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &ThisClass::EndAbility, Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled));
			return;
		}

		UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
		check(MyAbilityComponent);

		// When ability ends, consume target data and remove delegate
		MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).Remove(OnTargetDataReadyCallbackDelegateHandle);
		MyAbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());

		Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
}

void UASGameplayAbility_RangedWeapon::PerformLocalTargeting(FHitResult& OutHit)
{
	AActor* const AvatarPawn = GetAvatarActorFromActorInfo();

	FCollisionQueryParams Params(SCENE_QUERY_STAT(AbilityLineTrace), true, AvatarPawn);
	Params.bReturnPhysicalMaterial = true;
	Params.bTraceComplex = true;
	Params.AddIgnoredActor(AvatarPawn);

	// Get viewpoint (from player controller / camera)
	FVector ViewLoc = AvatarPawn->GetActorLocation();
	FRotator ViewRot = AvatarPawn->GetActorRotation();

	// If pawn has controller, get controller view
	if (AController* Controller = AvatarPawn->GetInstigatorController())
	{
		Controller->GetPlayerViewPoint(ViewLoc, ViewRot);
	}

	const FVector Start = ViewLoc;
	const FVector End = Start + (ViewRot.Vector() * 10000.0f);


	UWorld* World = AvatarPawn->GetWorld();
	if (!World) return;

	bool bHit = World->LineTraceSingleByChannel(OutHit, Start, End, COLLISION_PROJECTILE, Params);
	// Debug draw
	/*DrawDebugLine(World, Start, End, bHit ? LineColor : FColor::Green, false, 2.0f, 0, 2.0f);
	if (bHit)
	{
		DrawDebugSphere(World, OutHit.ImpactPoint, 8.0f, 8, FColor::Yellow, false, 2.0f);
	}*/

}

void UASGameplayAbility_RangedWeapon::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag)
{
	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilityComponent);

	if (const FGameplayAbilitySpec* AbilitySpec = MyAbilityComponent->FindAbilitySpecFromHandle(CurrentSpecHandle))
	{
		FScopedPredictionWindow ScopedPrediction(MyAbilityComponent);

		// Take ownership of the target data to make sure no callbacks into game code invalidate it out from under us
		FGameplayAbilityTargetDataHandle LocalTargetDataHandle(MoveTemp(const_cast<FGameplayAbilityTargetDataHandle&>(InData)));

		const bool bShouldNotifyServer = CurrentActorInfo->IsLocallyControlled() && !CurrentActorInfo->IsNetAuthority();
		if (bShouldNotifyServer)
		{
			MyAbilityComponent->CallServerSetReplicatedTargetData(CurrentSpecHandle,CurrentActivationInfo.GetActivationPredictionKey(), LocalTargetDataHandle, ApplicationTag, MyAbilityComponent->ScopedPredictionKey);
		}

		const bool bIsTargetDataValid = true;

		bool bProjectileWeapon = false;
#if WITH_SERVER_CODE
		if (!bProjectileWeapon)
		{
			if (AController* Controller = GetControllerFromActorInfo())
			{
				if (Controller->GetLocalRole() == ROLE_Authority) // check authority at listen server
				{
					TArray<uint8> HitReplaces;
					for (uint8 i = 0; (i < LocalTargetDataHandle.Num()) && (i < 255); ++i)
					{
						if (FGameplayAbilityTargetData_SingleTargetHit* SingleTargetHit = static_cast<FGameplayAbilityTargetData_SingleTargetHit*>(LocalTargetDataHandle.Get(i)))
						{
							if (SingleTargetHit->bHitReplaced)
							{
								HitReplaces.Add(i);
							}
						}
					}
				}
			}
		}
#endif

		if (bIsTargetDataValid && CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
		{
			OnRangedWeaponTargetDataReady(LocalTargetDataHandle);
		}
		
		// We've processed the data
		MyAbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
	}
}

void UASGameplayAbility_RangedWeapon::StartRangedWeaponTargeting()
{
	check(CurrentActorInfo);
	
	AActor* AvatarActor = CurrentActorInfo->AvatarActor.Get();
	check(AvatarActor);
	
	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilityComponent);

	FScopedPredictionWindow ScopedpredictionWindow(MyAbilityComponent , CurrentActivationInfo.GetActivationPredictionKey());

	FHitResult FoundHit;
	PerformLocalTargeting(/*out*/ FoundHit);

	//Fill target data from the hit results
	FGameplayAbilityTargetDataHandle TargetData;
	

	FGameplayAbilityTargetData_SingleTargetHit* NewTargetData = new FGameplayAbilityTargetData_SingleTargetHit();
	NewTargetData->HitResult = FoundHit;

	TargetData.Add(NewTargetData);



	// Process the target data immediately
	OnTargetDataReadyCallback(TargetData, FGameplayTag());
}
