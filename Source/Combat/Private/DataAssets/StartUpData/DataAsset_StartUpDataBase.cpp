// Zhang All Rights Reserved.


#include "DataAssets/StartUpData/DataAsset_StartUpDataBase.h"

#include "AbilitySystem/Abilities/CombatGameplayAbility.h"
#include "AbilitySystem/CombatAbilitySystemComponent.h"
#include "GameplayEffect.h"

void UDataAsset_StartUpDataBase::GrantAbilities(const TArray<TSubclassOf<UCombatGameplayAbility>>& InAbilitiesToGive, UCombatAbilitySystemComponent* InASCToGive, int32 ApplyLevel)
{
	if (InAbilitiesToGive.IsEmpty())
	{
		return;
	}

	for (const TSubclassOf<UCombatGameplayAbility>& AbilityClass : InAbilitiesToGive)
	{
		if (!AbilityClass)
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(AbilityClass, ApplyLevel, -1, InASCToGive->GetAvatarActor());
		InASCToGive->GiveAbility(AbilitySpec);
	}
}

void UDataAsset_StartUpDataBase::GrantEffects(const TArray<TSubclassOf<UGameplayEffect>>& InEffectsToGive, UCombatAbilitySystemComponent* InASCToGive, int32 ApplyLevel)
{
	if (InEffectsToGive.IsEmpty())
	{
		return;
	}

	for (const TSubclassOf<UGameplayEffect>& EffectClass : InEffectsToGive)
	{
		if (!EffectClass)
		{
			continue;
		}

		UGameplayEffect* EffectCDO = EffectClass->GetDefaultObject<UGameplayEffect>();// EffectCDO -> Effect Class Default object
		InASCToGive->ApplyGameplayEffectToSelf(EffectCDO, ApplyLevel, InASCToGive->MakeEffectContext());
	}
}

void UDataAsset_StartUpDataBase::GiveDataToAbilitySystemComponent(UCombatAbilitySystemComponent* InASCToGive, int32 ApplyLevel)
{
	check(InASCToGive);

	GrantAbilities(ActivateOnGivenAbilitieSet, InASCToGive, ApplyLevel);
	GrantAbilities(ReactiveAbilitieSet, InASCToGive, ApplyLevel);

	GrantEffects(StartUpGameplayEffectSet, InASCToGive, ApplyLevel);
}
