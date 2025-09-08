// Zhang All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AnimInstances/CombatCharacterAnimInstance.h"
#include "CombatHeroAnimInstance.generated.h"

class ACombatHeroCharacter;

/**
 * 
 */
UCLASS()
class COMBAT_API UCombatHeroAnimInstance : public UCombatCharacterAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

protected:
	// This parameter should change for there is a ptr for owning character in parent class. But animation will use this parameter, should change it later.
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|References")
	ACombatHeroCharacter* OwningHeroCharacter;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|LocomotionData")
	bool bShouldEnterRelaxState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimData|LocomotionData")
	float EnterRelaxStateThreshold = 5.f;

	float IdleElpasedTime;

private:
	void CastOwningBaseCharacterToHeroCharacter();
	void UpdatebShouldEnterRelaxState(const float& DeltaSeconds);
	
};
