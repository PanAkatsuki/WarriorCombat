// Zhang All Rights Reserved.


#include "AbilitySystem/Abilities/HeroAbility_Roll.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Characters/CombatHeroCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "MotionWarpingComponent.h"
#include "TimerManager.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "CombatGameplayTags.h"
#include "CombatFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystem/CombatAbilitySystemComponent.h"

#include "CombatDebugHelper.h"


UHeroAbility_Roll::UHeroAbility_Roll()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// Set LatentInfo
	RollLatentInfo.CallbackTarget = this;
	RollLatentInfo.ExecutionFunction = FName("OnDelayFinished");
	RollLatentInfo.Linkage = 0;
	RollLatentInfo.UUID = GetTypeHash(FName("OnDelayFinished"));
}

void UHeroAbility_Roll::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	PerfectRollTimerDelegate.BindUObject(this, &ThisClass::OnPerfectRollTimerEnd);
	GetWorld()->GetTimerManager().SetTimer(
		PerfectRollTimerHandle,
		PerfectRollTimerDelegate,
		0.6f,
		false
	);

	UCombatFunctionLibrary::AddGameplayTagToActorIfNone(GetHeroCharacterFromActorInfo(), CombatGameplayTags::Shared_Status_Invincible);

	WaitPerfectRollTaskHandle = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		CombatGameplayTags::Player_Event_PerfectRoll
	);

	WaitPerfectRollTaskHandle->EventReceived.AddUniqueDynamic(this, &ThisClass::OnEventReceived);

	WaitPerfectRollTaskHandle->ReadyForActivation();
	
	ComputeRollDiractionAndDistance();

	check(GetWorld());
	UKismetSystemLibrary::Delay(
		GetHeroCharacterFromActorInfo(), 
		0.02f,
		RollLatentInfo);
}

void UHeroAbility_Roll::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	UCombatFunctionLibrary::RemoveGameplayTagFromActorIfFound(GetHeroCharacterFromActorInfo(), CombatGameplayTags::Shared_Status_Invincible);
}

void UHeroAbility_Roll::OnDelayFinished()
{
	SetPlayMontageTask(this, FName("RollMontageTask"), FindMontageToPlayByRandom(AnimMontagesMap));
}

void UHeroAbility_Roll::ComputeRollDiractionAndDistance()
{
	FVector RollingDirection = UKismetMathLibrary::Normal(GetHeroCharacterFromActorInfo()->GetLastMovementInputVector());
	GetHeroCharacterFromActorInfo()->GetMotionWarpingComponent()->AddOrUpdateWarpTargetFromLocationAndRotation(
		FName("RollingDirection"),
		FVector(),
		UKismetMathLibrary::MakeRotFromX(RollingDirection)
	);

	//
	check(GetWorld());

	FVector HeroLocation = GetHeroCharacterFromActorInfo()->GetActorLocation();
	float RollingDistance = RollDistanceScalableFloat.GetValueAtLevel(GetAbilityLevel());
	FVector StartVector = HeroLocation + UKismetMathLibrary::Multiply_VectorFloat(RollingDirection, RollingDistance);

	FVector EndVector = GetHeroCharacterFromActorInfo()->GetActorUpVector() * -1.f * 500.f + StartVector;

	TArray<FHitResult> OutHits;

	UKismetSystemLibrary::LineTraceMultiForObjects(
		GetWorld(),
		StartVector,
		EndVector,
		ObjectTypes,
		false,
		TArray<AActor*>(),
		EDrawDebugTrace::None,
		OutHits,
		true
	);

	check(OutHits.Num());
	if (OutHits.GetData()->bBlockingHit)
	{
		GetHeroCharacterFromActorInfo()->GetMotionWarpingComponent()->AddOrUpdateWarpTargetFromLocation(
			FName("RollTargetLocation"),
			OutHits.GetData()->ImpactPoint
		);
	}
	else
	{
		GetHeroCharacterFromActorInfo()->GetMotionWarpingComponent()->RemoveWarpTarget(
			FName("RollTargetLocation")
		);
	}
}

void UHeroAbility_Roll::OnMontageCompleted()
{
	//Debug::Print(TEXT("MontageCompleted"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UHeroAbility_Roll::OnMontageBlendOut()
{
	//Debug::Print(TEXT("MontageBlendOut"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UHeroAbility_Roll::OnMontageInterrupted()
{
	//Debug::Print(TEXT("MontageInterrupted"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}

void UHeroAbility_Roll::OnMontageCancelled()
{
	//Debug::Print(TEXT("MontageCancelled"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}

void UHeroAbility_Roll::OnEventReceived(FGameplayEventData InEventData)
{
	//Debug::Print(TEXT("PerfectRoll"));
	// Time Slow
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), TimeDilation);

	ACombatHeroCharacter* PlayerCharacter = GetHeroCharacterFromActorInfo();
	
	UCombatFunctionLibrary::AddGameplayTagToActorIfNone(PlayerCharacter, CombatGameplayTags::Player_Status_TimeSlow);
	PlayerCharacter->CustomTimeDilation = 1.0f / TimeDilation;

	GetWorld()->GetTimerManager().SetTimer(
		GetHeroCharacterFromActorInfo()->TimeStopTimerHandle,
		GetHeroCharacterFromActorInfo()->TimeStopTimerDelegate,
		TimeSlowTime,
		false
	);

	// Play Gameplay Cue
	FGameplayCueParameters GameplayCueParameters;
	GameplayCueParameters.NormalizedMagnitude = 1.f;
	GameplayCueParameters.Location = CurrentActorInfo->AvatarActor.Get()->GetActorLocation();

	CurrentActorInfo->AbilitySystemComponent->ExecuteGameplayCue(TimeSlowGameplayCueTag, GameplayCueParameters);
}

void UHeroAbility_Roll::OnPerfectRollTimerEnd()
{
	//Debug::Print(TEXT("TimerEnd"));
	UCombatFunctionLibrary::RemoveGameplayTagFromActorIfFound(GetHeroCharacterFromActorInfo(), CombatGameplayTags::Shared_Status_Invincible);

	WaitPerfectRollTaskHandle->EndTask();
}
