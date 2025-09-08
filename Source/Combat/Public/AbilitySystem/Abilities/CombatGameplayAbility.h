// Zhang All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"

#include "CombatTypes/CombatEnumTypes.h"

#include "CombatGameplayAbility.generated.h"


class UPawnFightComponent;
class UCombatAbilitySystemComponent;

UENUM(BlueprintType)
enum class ECombatAbilityActivationPolicy : uint8
{
	OnTriggered,
	OnGiven
};

/**
 * 
 */
UCLASS()
class COMBAT_API UCombatGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Ability")
	TMap<int32, UAnimMontage*> AnimMontagesMap;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Ability")
	ECombatAbilityActivationPolicy AbilityActivationPolicy = ECombatAbilityActivationPolicy::OnTriggered;

protected:
	//~ Begin UGameplayAbility Interface
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~ End UGameplayAbility Interface

	// Set Play Montage Task
	void SetPlayMontageTask(UGameplayAbility* OwningAbility, FName TaskInstanceName, UAnimMontage* MontageToPlay, float Rate = 1.0f, FName StartSection = NAME_None, bool bStopWhenAbilityEnds = true, float AnimRootMotionTranslationScale = 1.0f, float StartTimeSeconds = 0.0f, bool bAllowInterruptAfterBlendOut = false);
	virtual UAnimMontage* FindMontageToPlayByRandom(TMap<int32, UAnimMontage*>& InAnimMontagesMap);

private:
	// Callback function fro Play Montage Task
	// In this class, these function should be empty
	// If ability class call function SetPlayMontageTask, complete these function in child class
	void OnMontageCompleted();
	void OnMontageBlendOut();
	void OnMontageInterrupted();
	void OnMontageCancelled();

protected:
	// Set Wait Event Task
	void SetWaitMontageEventTask(UGameplayAbility* OwningAbility, FGameplayTag WaitEventTag, AActor* OptionalExternalTarget = nullptr, bool OnlyTriggerOnce = false, bool OnlyMatchExact = true);

private:
	// Callback function fro wait event task
	// In this class, this function should be empty
	// If ability class call function SetWaitMontageEventTask, complete OnEventReceived function in child class
	void OnEventReceived(FGameplayEventData InEventData);

protected:
	UPawnFightComponent* GetPawnFightComponentFromActorInfo() const;
	UCombatAbilitySystemComponent* GetCombatAbilitySystemComponentFromActorInfo() const;
	FActiveGameplayEffectHandle NativeApplyEffectSpecHandleToTarget(AActor* TargetActor, const FGameplayEffectSpecHandle& InSpecHandle);
	void ApplyGameplayEffectSpecHandleToHitResults(const FGameplayEffectSpecHandle& InSpecHandle, const TArray<FHitResult>& InHitResults);

private:
	void TryActivateOnGivenAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec);
	void ClearOnGivenAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpecHandle Handle);
};	
