// Zhang All Rights Reserved.


#include "AbilitySystem/Abilities/CombatGameplayAbility.h"

#include "AbilitySystem/CombatAbilitySystemComponent.h"
#include "Components/Fight/PawnFightComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "CombatFunctionLibrary.h"
#include "CombatGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

#include "CombatDebugHelper.h"


void UCombatGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	TryActivateOnGivenAbility(ActorInfo, Spec);
}

void UCombatGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	ClearOnGivenAbility(ActorInfo, Handle);
}

void UCombatGameplayAbility::SetPlayMontageTask(UGameplayAbility* OwningAbility, FName TaskInstanceName, UAnimMontage* MontageToPlay, float Rate, FName StartSection, bool bStopWhenAbilityEnds, float AnimRootMotionTranslationScale, float StartTimeSeconds, bool bAllowInterruptAfterBlendOut)
{
	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		OwningAbility,
		TaskInstanceName,
		MontageToPlay,
		Rate,
		StartSection,
		bStopWhenAbilityEnds,
		AnimRootMotionTranslationScale,
		StartTimeSeconds,
		bAllowInterruptAfterBlendOut
	);

	PlayMontageTask->OnCompleted.AddUniqueDynamic(this, &ThisClass::OnMontageCompleted);
	PlayMontageTask->OnBlendOut.AddUniqueDynamic(this, &ThisClass::OnMontageBlendOut);
	PlayMontageTask->OnInterrupted.AddUniqueDynamic(this, &ThisClass::OnMontageInterrupted);
	PlayMontageTask->OnCancelled.AddUniqueDynamic(this, &ThisClass::OnMontageCancelled);

	PlayMontageTask->ReadyForActivation();
}

UAnimMontage* UCombatGameplayAbility::FindMontageToPlayByRandom(TMap<int32, UAnimMontage*>& InAnimMontagesMap)
{
	check(InAnimMontagesMap.Num());

	int32 RandomInt = FMath::RandRange(1, InAnimMontagesMap.Num());
	UAnimMontage* const* MontagePtr = InAnimMontagesMap.Find(RandomInt);

	return MontagePtr ? *MontagePtr : nullptr;
}

void UCombatGameplayAbility::OnMontageCompleted()
{
}

void UCombatGameplayAbility::OnMontageBlendOut()
{
}

void UCombatGameplayAbility::OnMontageInterrupted()
{
}

void UCombatGameplayAbility::OnMontageCancelled()
{
}

void UCombatGameplayAbility::SetWaitMontageEventTask(UGameplayAbility* OwningAbility, FGameplayTag WaitEventTag, AActor* OptionalExternalTarget, bool OnlyTriggerOnce, bool OnlyMatchExact)
{
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		OwningAbility,
		WaitEventTag,
		OptionalExternalTarget,
		OnlyTriggerOnce,
		OnlyMatchExact
	);

	WaitEventTask->EventReceived.AddUniqueDynamic(this, &ThisClass::OnEventReceived);

	WaitEventTask->ReadyForActivation();
}

void UCombatGameplayAbility::OnEventReceived(FGameplayEventData InEventData)
{
}

UPawnFightComponent* UCombatGameplayAbility::GetPawnFightComponentFromActorInfo() const
{
	// Find component by interate. If actor has multiply UPawnFightComponent this function will NOT work correctly.
	UPawnFightComponent* PawnFightComponent = GetAvatarActorFromActorInfo()->FindComponentByClass<UPawnFightComponent>();
	check(PawnFightComponent);

	return PawnFightComponent;
}

UCombatAbilitySystemComponent* UCombatGameplayAbility::GetCombatAbilitySystemComponentFromActorInfo() const
{
	UCombatAbilitySystemComponent* CombatAbilitySystemComponent = Cast<UCombatAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent);
	check(CombatAbilitySystemComponent);

	return CombatAbilitySystemComponent;
}

FActiveGameplayEffectHandle UCombatGameplayAbility::NativeApplyEffectSpecHandleToTarget(AActor* TargetActor, const FGameplayEffectSpecHandle& InSpecHandle)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	checkf(TargetASC, TEXT("TargetASC is not valid."));

	checkf(InSpecHandle.IsValid(), TEXT("InSpecHandle is not valid."));
	return GetCombatAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*InSpecHandle.Data, TargetASC);
}

void UCombatGameplayAbility::ApplyGameplayEffectSpecHandleToHitResults(const FGameplayEffectSpecHandle& InSpecHandle, const TArray<FHitResult>& InHitResults)
{
	if (InHitResults.IsEmpty())
	{
		return;
	}

	for (const FHitResult& hit : InHitResults)
	{
		APawn* OwningPawn = CastChecked<APawn>(GetAvatarActorFromActorInfo());
		// HitPawn can be actor can't cast to APawn, ensure check if cast success
		APawn* HitPawn = Cast<APawn>(hit.GetActor());

		if (HitPawn && UCombatFunctionLibrary::IsTargetPawnHostile(OwningPawn, HitPawn))
		{
			FActiveGameplayEffectHandle ActiveGameplayEffectHandle = NativeApplyEffectSpecHandleToTarget(hit.GetActor(), InSpecHandle);

			if (ActiveGameplayEffectHandle.WasSuccessfullyApplied())
			{
				FGameplayEventData Data;
				Data.Instigator = OwningPawn;
				Data.Target = HitPawn;

				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(hit.GetActor(), CombatGameplayTags::Shared_Event_HitReact, Data);
			}
		}
	}
}

void UCombatGameplayAbility::TryActivateOnGivenAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	if (AbilityActivationPolicy == ECombatAbilityActivationPolicy::OnGiven)
	{
		if (ActorInfo && !Spec.IsActive())
		{
			ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
		}
	}
}

void UCombatGameplayAbility::ClearOnGivenAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpecHandle Handle)
{
	if (AbilityActivationPolicy == ECombatAbilityActivationPolicy::OnGiven)
	{
		if (ActorInfo)
		{
			ActorInfo->AbilitySystemComponent->ClearAbility(Handle);
		}
	}
}