// Zhang All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CombatBaseCharacter.h"

#include "GameplayTagContainer.h"

#include "CombatHeroCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UDataAsset_InputConfig;
struct FInputActionValue;
class UHeroFightComponent;
class UHeroUIComponent;
class ACombatEnemyCharacter;

/**
 * 
 */
UCLASS()
class COMBAT_API ACombatHeroCharacter : public ACombatBaseCharacter, public IPawnFightInterface
{
	GENERATED_BODY()
	
public:
	ACombatHeroCharacter();

protected:
	//~ Begin APawn Interface.
	// Load StartUpData
	// Called when character occupied by controller
	virtual void PossessedBy(AController* NewController) override;
	//~ End APawn Interface

	// Set Up Input Component
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:

#pragma region Components

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* AimCameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* AimCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fight", meta = (AllowPrivateAccess = "true"))
	UHeroFightComponent* HeroFightComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	UHeroUIComponent* HeroUIComponent;

#pragma endregion



#pragma region Inputs

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CharacterData", meta = (AllowPrivateAccess = "true"))
	UDataAsset_InputConfig* InputConfigDataAsset = nullptr;

#pragma endregion

	UPROPERTY()
	FVector2D SwitchDirection = FVector2D::ZeroVector;

public:
	// Timer
	FTimerHandle TimeStopTimerHandle;
	FTimerDelegate TimeStopTimerDelegate;

	TWeakObjectPtr<ACombatEnemyCharacter> TargetInEnemyCharacter;

#pragma region Inputs

	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_Look(const FInputActionValue& InputActionValue);

	void Input_SwitchTargetTriggered(const FInputActionValue& InputActionValue);
	void Input_SwitchTargetCompleted(const FInputActionValue& InputActionValue);

	void Input_PickUpStonesStarted(const FInputActionValue& InputActionValue);

	void Input_AbilityInputPressed(FGameplayTag InInputTag);
	void Input_AbilityInputReleased(FGameplayTag InInputTag);

#pragma endregion

public:
	FORCEINLINE UHeroFightComponent* GetHeroFightComponent() const { return HeroFightComponent; }
	FORCEINLINE UHeroUIComponent* GetHeroUIComponent() const { return HeroUIComponent; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE UCameraComponent* GetAimCamera() const { return AimCamera; }

	//~ Begin IPawnFightInterface Interface.
	virtual UPawnFightComponent* GetPawnFightComponent() const override;
	//~ End IPawnFightInterface Interface

	//~ Begin IPawnUIInterface Interface.
	virtual UPawnUIComponent* GetPawnUIComponent() const override;
	//~ End IPawnUIInterface Interface

public:
	UFUNCTION()
	void OnTimeStopTimerEnd();
};
