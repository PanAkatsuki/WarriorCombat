// Zhang All Rights Reserved.


#include "Characters/CombatBaseCharacter.h"

#include "AbilitySystem/CombatAbilitySystemComponent.h"
#include "AbilitySystem/CombatAttributeSet.h"
#include "MotionWarpingComponent.h"


// Sets default values
ACombatBaseCharacter::ACombatBaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	GetMesh()->bReceivesDecals = false;

	// Ability System Component
	CombatAbilitySystemComponent = CreateDefaultSubobject<UCombatAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	// AttributeSet
	CombatAttributeSet = CreateDefaultSubobject<UCombatAttributeSet>(TEXT("CombatAttributeSet"));

	// Motion Warping Component
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));

}

void ACombatBaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	ensureMsgf(!CharacterStartUpData.IsNull(), TEXT("Forget to assign start up data to %s"), *this->GetName());

	if (CombatAbilitySystemComponent)
	{
		CombatAbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

void ACombatBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

UAbilitySystemComponent* ACombatBaseCharacter::GetAbilitySystemComponent() const
{
	return Cast<UAbilitySystemComponent>(GetCombatAbilitySystemComponent());
}

UPawnUIComponent* ACombatBaseCharacter::GetPawnUIComponent() const
{
	return nullptr;
}

