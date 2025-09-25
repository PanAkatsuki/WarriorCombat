// Zhang All Rights Reserved.


#include "AbilitySystem/Abilities/HeroAbility_SpecialHeavy.h"

#include "CombatGameplayTags.h"
#include "Characters/CombatHeroCharacter.h"
#include "Components/UI/HeroUIComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/Fight/HeroFightComponent.h"

#include "CombatDebugHelper.h"


UHeroAbility_SpecialHeavy::UHeroAbility_SpecialHeavy()
{
}

void UHeroAbility_SpecialHeavy::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	SetPlayMontageTask(this, FName("SpecialHeavyMontageTask"), FindMontageToPlayByRandom(AnimMontagesMap));
	SetWaitMontageEventTask(this, CombatGameplayTags::Player_Event_AOE);

	CommitAbility(Handle, ActorInfo, ActivationInfo);
	GetHeroCharacterFromActorInfo()->GetHeroUIComponent()->OnAbilityCooldownBegin.Broadcast(
		CombatGameplayTags::InputTag_SpecialWeaponAbility_Heavy,
		GetCooldownTimeRemaining(),
		GetCooldownTimeRemaining()
	);
}

void UHeroAbility_SpecialHeavy::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

}

void UHeroAbility_SpecialHeavy::OnMontageCompleted()
{
	//Debug::Print(TEXT("MontageCompleted"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UHeroAbility_SpecialHeavy::OnMontageBlendOut()
{
	//Debug::Print(TEXT("MontageBlendOut"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UHeroAbility_SpecialHeavy::OnMontageInterrupted()
{
	//Debug::Print(TEXT("MontageInterrupted"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}

void UHeroAbility_SpecialHeavy::OnMontageCancelled()
{
	//Debug::Print(TEXT("MontageCancelled"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}

void UHeroAbility_SpecialHeavy::OnEventReceived(FGameplayEventData InEventData)
{
	ensureMsgf(!ObjectTypes.IsEmpty(), TEXT("ObjectTypes is empty, Should set ObjectTypes!"));

	FVector ForwardVector = GetHeroCharacterFromActorInfo()->GetActorForwardVector();

	TArray<FHitResult> Hits;

	check(GetWorld());
	UKismetSystemLibrary::BoxTraceMultiForObjects(
		GetWorld(),
		GetHeroCharacterFromActorInfo()->GetActorLocation(),
		ForwardVector * 160.f + GetHeroCharacterFromActorInfo()->GetActorLocation(),
		FVector(200.f, 200.f, 200.f),
		UKismetMathLibrary::MakeRotFromX(ForwardVector),
		ObjectTypes,
		false,
		TArray<AActor*, FDefaultAllocator>(),
		EDrawDebugTrace::None,
		Hits,
		true
	);

	FGameplayEffectSpecHandle GameplayEffectSpecHandle = MakeHeroDamageEffectsSpecHandle(
		EffectSet.DealDamageEffectClass,
		GetHeroFightComponentFromActorInfo()->GetHeroCurrentEquippedWeaponDamageAtLevel(GetAbilityLevel()),
		CombatGameplayTags::Player_SetByCaller_AttackType_SpecialWeaponAbility,
		1
	);

	ApplyGameplayEffectSpecHandleToHitResults(GameplayEffectSpecHandle, Hits);
}