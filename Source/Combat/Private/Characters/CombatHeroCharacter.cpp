// Zhang All Rights Reserved.


#include "Characters/CombatHeroCharacter.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "DataAssets/Input/DataAsset_InputConfig.h"
#include "Components/Input/CombatInputComponent.h"
#include "CombatGameplayTags.h"
#include "AbilitySystem/CombatAbilitySystemComponent.h"
#include "DataAssets/StartUpData/DataAsset_StartUpDataBase.h"
#include "Components/Fight/HeroFightComponent.h"
#include "Components/UI/HeroUIComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameModes/CombatBaseGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "CombatFunctionLibrary.h"

#include "CombatDebugHelper.h"


ACombatHeroCharacter::ACombatHeroCharacter()
{
	// Capsule Component
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	// Pawn Parameter
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Camera Component
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 200.f;
	CameraBoom->SocketOffset = FVector(0.f, 60.f, 80.f);
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	AimCameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("AimCameraBoom"));
	AimCameraBoom->SetupAttachment(GetRootComponent());
	AimCameraBoom->TargetArmLength = 100.f;
	AimCameraBoom->SocketOffset = FVector(0.f, 60.f, 80.f);
	AimCameraBoom->bUsePawnControlRotation = true;

	AimCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("AnimCamera"));
	AimCamera->SetupAttachment(AimCameraBoom, USpringArmComponent::SocketName);
	AimCamera->bUsePawnControlRotation = false;
	AimCamera->bAutoActivate = false;

	// Character Movement Component
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = 400.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Hero Fight Component
	HeroFightComponent = CreateDefaultSubobject<UHeroFightComponent>(TEXT("HeroFightComponent"));

	// Hero UI Component
	HeroUIComponent = CreateDefaultSubobject<UHeroUIComponent>(TEXT("HeroUIComponent"));

	// Timer
	TimeStopTimerDelegate.BindUObject(this, &ThisClass::OnTimeStopTimerEnd);
}

void ACombatHeroCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Initialize hero startup data
	if (CharacterStartUpData.IsNull())
	{
		return;
	}

	if (UDataAsset_StartUpDataBase* LoadedData = CharacterStartUpData.LoadSynchronous())
	{
		int32 AbilityApplyLevel = 1;

		//Set AbilityApplyLevel by GameDifficulty
		if (ACombatBaseGameMode* BaseGameMode = GetWorld()->GetAuthGameMode<ACombatBaseGameMode>())
		{
			switch (BaseGameMode->GetCurrentGameDifficulty())
			{
			case ECombatGameDifficulty::Easy:
				AbilityApplyLevel = 1;
				//Debug::Print(TEXT("Easy"));
				break;
			case ECombatGameDifficulty::Normal:
				AbilityApplyLevel = 2;
				//Debug::Print(TEXT("Normal"));
				break;
			case ECombatGameDifficulty::Hard:
				AbilityApplyLevel = 3;
				//Debug::Print(TEXT("Hard"));
				break;
			case ECombatGameDifficulty::Insane:
				AbilityApplyLevel = 4;
				//Debug::Print(TEXT("Insane"));
				break;
			default:
				break;
			}
		}

		LoadedData->GiveDataToAbilitySystemComponent(CombatAbilitySystemComponent, AbilityApplyLevel);
	}
}

void ACombatHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	checkf(InputConfigDataAsset, TEXT("InputConfigDataAsset is null, should set it in editor!"));

	ULocalPlayer* LocalPlayer = GetController<APlayerController>()->GetLocalPlayer();
	UEnhancedInputLocalPlayerSubsystem* EnhancedInputLocalPlayerSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	check(EnhancedInputLocalPlayerSubsystem);

	// AddMappingContext to EnhancedInputLocalPlayerSubsystem
	EnhancedInputLocalPlayerSubsystem->AddMappingContext(InputConfigDataAsset->DefaultInputMappingContext, 0);

	// BindNativeInputAction
	UCombatInputComponent* CombatInputComponent = CastChecked<UCombatInputComponent>(PlayerInputComponent);

	CombatInputComponent->BindNativeInputAction(InputConfigDataAsset, CombatGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);
	CombatInputComponent->BindNativeInputAction(InputConfigDataAsset, CombatGameplayTags::InputTag_Look, ETriggerEvent::Triggered, this, &ThisClass::Input_Look);

	CombatInputComponent->BindNativeInputAction(InputConfigDataAsset, CombatGameplayTags::InputTag_SwitchTarget, ETriggerEvent::Triggered, this, &ThisClass::Input_SwitchTargetTriggered);
	CombatInputComponent->BindNativeInputAction(InputConfigDataAsset, CombatGameplayTags::InputTag_SwitchTarget, ETriggerEvent::Completed, this, &ThisClass::Input_SwitchTargetCompleted);

	CombatInputComponent->BindNativeInputAction(InputConfigDataAsset, CombatGameplayTags::InputTag_PickUp_Stones, ETriggerEvent::Started, this, &ThisClass::Input_PickUpStonesStarted);

	// BindAbilityInputAction
	CombatInputComponent->BindAbilityInputAction(InputConfigDataAsset, this, &ThisClass::Input_AbilityInputPressed, &ThisClass::Input_AbilityInputReleased);
}

void ACombatHeroCharacter::Input_Move(const FInputActionValue& InputActionValue)
{
	const FVector2D MovementVector = InputActionValue.Get<FVector2D>();
	const FRotator MovementRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);

	if (MovementVector.Y != 0.f)
	{
		const FVector ForwardDirection = MovementRotation.RotateVector(FVector::ForwardVector);
		AddMovementInput(ForwardDirection, MovementVector.Y);
	}

	if (MovementVector.X != 0.f)
	{
		const FVector RightDirection = MovementRotation.RotateVector(FVector::RightVector);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ACombatHeroCharacter::Input_Look(const FInputActionValue& InputActionValue)
{
	const FVector2D LookAxisVector = InputActionValue.Get<FVector2D>();

	if (LookAxisVector.X != 0.f)
	{
		AddControllerYawInput(LookAxisVector.X);
	}

	if (LookAxisVector.Y != 0.f)
	{
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ACombatHeroCharacter::Input_SwitchTargetTriggered(const FInputActionValue& InputActionValue)
{
	SwitchDirection = InputActionValue.Get<FVector2D>();
}

void ACombatHeroCharacter::Input_SwitchTargetCompleted(const FInputActionValue& InputActionValue)
{
	if (SwitchDirection.X >= 0.8f)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			this,
			CombatGameplayTags::Player_Event_SwithchTarget_Right,
			FGameplayEventData()
		);
	}

	if (SwitchDirection.X <= -0.8f)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			this,
			CombatGameplayTags::Player_Event_SwithchTarget_Left,
			FGameplayEventData()
		);
	}
}

void ACombatHeroCharacter::Input_PickUpStonesStarted(const FInputActionValue& InputActionValue)
{
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		this,
		CombatGameplayTags::Player_Event_ConsumeStones,
		FGameplayEventData()
	);
}

void ACombatHeroCharacter::Input_AbilityInputPressed(FGameplayTag InInputTag)
{
	CombatAbilitySystemComponent->OnAbilityInputPressed(InInputTag);
}

void ACombatHeroCharacter::Input_AbilityInputReleased(FGameplayTag InInputTag)
{
	CombatAbilitySystemComponent->OnAbilityInputReleased(InInputTag);
}

UPawnFightComponent* ACombatHeroCharacter::GetPawnFightComponent() const
{
	return HeroFightComponent;
}

UPawnUIComponent* ACombatHeroCharacter::GetPawnUIComponent() const
{
	return HeroUIComponent;
}

void ACombatHeroCharacter::OnTimeStopTimerEnd()
{
	//Debug::Print(TEXT("OnTimeStopTimerEnd"));
	UCombatFunctionLibrary::RemoveGameplayTagFromActorIfFound(this, CombatGameplayTags::Player_Status_TimeSlow);
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);

	CustomTimeDilation = 1.0f;
}