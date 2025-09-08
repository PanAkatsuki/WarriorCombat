// Zhang All Rights Reserved.


#include "AbilitySystem/Abilities/SharedAbility_SpawnWeapon.h"

#include "Items/Weapons/CombatWeaponBase.h"
#include "Components/Fight/PawnFightComponent.h"


USharedAbility_SpawnWeapon::USharedAbility_SpawnWeapon()
{
	AbilityActivationPolicy = ECombatAbilityActivationPolicy::OnGiven;
}

void USharedAbility_SpawnWeapon::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACombatWeaponBase* Weapon = SpawnWeapon(WeaponClassToSpawn);
	AttachWeaponToSocket(Weapon, GetOwningComponentFromActorInfo(), SocketNameToAttachTo, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, EAttachmentRule::KeepWorld, true);
	RegisterWeapon(Weapon, WeaponTagToRigister, bRegisterAsEquippedWeapon);

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void USharedAbility_SpawnWeapon::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
}

void USharedAbility_SpawnWeapon::AttachWeaponToSocket(ACombatWeaponBase* InWeapon, USkeletalMeshComponent* InSkeletalMeshComponent, FName InSocketNameToAttachTo, EAttachmentRule LocationRule, EAttachmentRule RotationRule, EAttachmentRule ScaleRule, bool WeldSimulatedBodies)
{
	IAttachWeaponInterface::AttachWeaponToSocket(InWeapon, InSkeletalMeshComponent, InSocketNameToAttachTo, LocationRule, RotationRule, ScaleRule, WeldSimulatedBodies);
}

ACombatWeaponBase* USharedAbility_SpawnWeapon::SpawnWeapon(TSubclassOf<ACombatWeaponBase>& InWeaponToSpawn)
{
	// Spawn Weapon
	check(GetWorld());

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetAvatarActorFromActorInfo();
	SpawnParams.Instigator = Cast<APawn>(GetAvatarActorFromActorInfo());
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ACombatWeaponBase* SpawnedWeapon = GetWorld()->SpawnActor<ACombatWeaponBase>(InWeaponToSpawn, FVector(), FRotator(), SpawnParams);
	check(SpawnedWeapon);

	return SpawnedWeapon;
}

void USharedAbility_SpawnWeapon::RegisterWeapon(ACombatWeaponBase* InWeapon, FGameplayTag InWeaponTagToRigister, bool InbRegisterAsEquippedWeapon)
{
	GetPawnFightComponentFromActorInfo()->RegisterSpawnedWeapon(InWeaponTagToRigister, InWeapon, InbRegisterAsEquippedWeapon);

	InWeapon->GetWeaponMesh()->SetScalarParameterValueOnMaterials(FName("DissolveAmount"), 1.0f);
}