// Zhang All Rights Reserved.


#include "DataAssets/Input/DataAsset_InputConfig.h"

#include "CombatDebugHelper.h"

UInputAction* UDataAsset_InputConfig::FindNativeInputActionByTag(const FGameplayTag& InInputTag) const
{
	for (const FCombatInputActionConfig& InputActionConfig : NativeInputActionConfigSet)
	{
		if (!InputActionConfig.IsValid())
		{
			Debug::Print(TEXT("There is a invalid input action config, please check the input config in editor."));
			continue;
		}

		if (InputActionConfig.InputTag == InInputTag && InputActionConfig.InputAction)
		{
			return InputActionConfig.InputAction;
		}
	}

	return nullptr;
}