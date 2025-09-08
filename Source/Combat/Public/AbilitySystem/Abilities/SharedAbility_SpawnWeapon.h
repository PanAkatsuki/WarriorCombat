// Zhang All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CombatGameplayAbility.h"

#include "Interfaces/AttachWeaponInterface.h"

#include "SharedAbility_SpawnWeapon.generated.h"

class ACombatWeaponBase;

/**
 * 
 */
UCLASS()
class COMBAT_API USharedAbility_SpawnWeapon : public UCombatGameplayAbility, public IAttachWeaponInterface
{
	GENERATED_BODY()

public:
	USharedAbility_SpawnWeapon();

private:
	UPROPERTY(EditDefaultsOnly, Category = "SpawnWeapon")
	TSubclassOf<ACombatWeaponBase> WeaponClassToSpawn;

	UPROPERTY(EditDefaultsOnly, Category = "SpawnWeapon")
	FName SocketNameToAttachTo;

	UPROPERTY(EditDefaultsOnly, Category = "SpawnWeapon")
	FGameplayTag WeaponTagToRigister;

	UPROPERTY(EditDefaultsOnly, Category = "SpawnWeapon")
	bool bRegisterAsEquippedWeapon = false;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	//~ Begin IAttachWeaponInterface Interface ~//
	virtual void AttachWeaponToSocket(ACombatWeaponBase* InWeapon, USkeletalMeshComponent* InSkeletalMeshComponent, FName InSocketNameToAttachTo, EAttachmentRule LocationRule, EAttachmentRule RotationRule, EAttachmentRule ScaleRule, bool WeldSimulatedBodies) override;
	//~ End IAttachWeaponInterface Interface ~//

private:
	ACombatWeaponBase* SpawnWeapon(TSubclassOf<ACombatWeaponBase>& InWeaponToSpawn);
	void RegisterWeapon(ACombatWeaponBase* InWeapon, FGameplayTag InWeaponTagToRigister, bool InbRegisterAsEquippedWeapon);
};
