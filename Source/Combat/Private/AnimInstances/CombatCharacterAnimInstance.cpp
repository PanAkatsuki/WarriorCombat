// Zhang All Rights Reserved.


#include "AnimInstances/CombatCharacterAnimInstance.h"

#include "Characters/CombatBaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"

#include "CombatDebugHelper.h"


void UCombatCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	//This function will be called in editor, so here should not use CastCheck.
	OwningCharacter = Cast<ACombatBaseCharacter>(TryGetPawnOwner());

	if (OwningCharacter)
	{
		OwningMovementComponent = OwningCharacter->GetCharacterMovement();
	}
	else
	{
		//Debug::Print(TEXT("In UCombatCharacterAnimInstance::NativeInitializeAnimation, OwningCharacter cast failed!"));
	}
}

void UCombatCharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	if (!OwningCharacter || !OwningMovementComponent)
	{
		//This function will be called in editor.
		//Debug::Print(TEXT("In UCombatCharacterAnimInstance::NativeThreadSafeUpdateAnimation, OwningCharacter or OwningMovementComponent is null!"));
		return;
	}

	GroundSpeed = OwningCharacter->GetVelocity().Size2D();

	bHasAcceleration = OwningMovementComponent->GetCurrentAcceleration().SizeSquared2D() > 0.f;

	LocomotionDirection = UKismetAnimationLibrary::CalculateDirection(
		OwningCharacter->GetVelocity(), 
		OwningCharacter->GetActorRotation()
	);
}
