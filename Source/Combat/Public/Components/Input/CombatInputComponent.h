// Zhang All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"

#include "DataAssets/Input/DataAsset_InputConfig.h"

#include "CombatInputComponent.generated.h"

/**
 * 
 */
UCLASS()
class COMBAT_API UCombatInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	// BindNativeInputAction() should set TriggerEvent for all UInputAction
	template<class UserObject, typename CallbackFunction>
	void BindNativeInputAction(const UDataAsset_InputConfig* InInputConfig, const FGameplayTag& InInputTag, ETriggerEvent InTriggerEvent, UserObject* InContextObject, CallbackFunction InCallbackFunction);
	
	// Ability only have two TriggerEvent, for this BindAbilityInputAction() can bind all Ability in ONE function
	template<class UserObject, typename CallbackFunction>
	void BindAbilityInputAction(const UDataAsset_InputConfig* InInputConfig, UserObject* InContextObject, CallbackFunction InInputPressedFunction, CallbackFunction InInputReleasedFunction);
};

template<class UserObject, typename CallbackFunction>
inline void UCombatInputComponent::BindNativeInputAction(const UDataAsset_InputConfig* InInputConfig, const FGameplayTag& InInputTag, ETriggerEvent InTriggerEvent, UserObject* InContextObject, CallbackFunction InCallbackFunction)
{
	checkf(InInputConfig, TEXT("InInputConfig is null, should set in editor!"));

	if (UInputAction* FoundAction = InInputConfig->FindNativeInputActionByTag(InInputTag))
	{
		BindAction(FoundAction, InTriggerEvent, InContextObject, InCallbackFunction);
	}
}

template<class UserObject, typename CallbackFunction>
inline void UCombatInputComponent::BindAbilityInputAction(const UDataAsset_InputConfig* InInputConfig, UserObject* InContextObject, CallbackFunction InInputPressedFunction, CallbackFunction InInputReleasedFunction)
{
	checkf(InInputConfig, TEXT("InInputConfig is null, should set in editor!."));

	for (const FCombatInputActionConfig& AbilityInputActionConfig : InInputConfig->AbilityInputActionConfigSet)
	{
		if (!AbilityInputActionConfig.IsValid())
		{
			continue;
		}

		BindAction(AbilityInputActionConfig.InputAction, ETriggerEvent::Started, InContextObject, InInputPressedFunction, AbilityInputActionConfig.InputTag);
		BindAction(AbilityInputActionConfig.InputAction, ETriggerEvent::Completed, InContextObject, InInputReleasedFunction, AbilityInputActionConfig.InputTag);
	}
}
