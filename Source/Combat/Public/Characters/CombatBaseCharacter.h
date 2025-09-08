// Zhang All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "AbilitySystemInterface.h"
#include "Interfaces/PawnFightInterface.h"
#include "Interfaces/PawnUIInterface.h"

#include "CombatBaseCharacter.generated.h"

class UCombatAbilitySystemComponent;
class UCombatAttributeSet;
class UDataAsset_StartUpDataBase;
class UMotionWarpingComponent;

UCLASS()
class COMBAT_API ACombatBaseCharacter : public ACharacter, public IAbilitySystemInterface, public IPawnUIInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACombatBaseCharacter();

protected:
	//~ Begin APawn Interface.
	// Load StartUpData
	// Called when character occupied by controller
	virtual void PossessedBy(AController* NewController) override;
	//~ End APawn Interface

public:	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	UCombatAbilitySystemComponent* CombatAbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	UCombatAttributeSet* CombatAttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MotionWarping")
	UMotionWarpingComponent* MotionWarpingComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CharacterData")
	TSoftObjectPtr<UDataAsset_StartUpDataBase> CharacterStartUpData;

public:
	FORCEINLINE UCombatAbilitySystemComponent* GetCombatAbilitySystemComponent() const { return CombatAbilitySystemComponent; }
	FORCEINLINE UCombatAttributeSet* GetCombatAttributeSet() const { return CombatAttributeSet; }
	FORCEINLINE UMotionWarpingComponent* GetMotionWarpingComponent() const { return MotionWarpingComponent; }

	//~ Begin IAbilitySystemInterface Interface.
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ End IAbilitySystemInterface Interface

	//~ Begin IPawnUIInterface Interface.
	virtual UPawnUIComponent* GetPawnUIComponent() const override;
	//~ End IPawnUIInterface Interface

};
