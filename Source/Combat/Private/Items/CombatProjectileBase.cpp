// Zhang All Rights Reserved.


#include "Items/CombatProjectileBase.h"

#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "CombatFunctionLibrary.h"
#include "CombatGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/AssetManager.h"
#include "Components/AudioComponent.h"

#include "CombatDebugHelper.h"


// Sets default values
ACombatProjectileBase::ACombatProjectileBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ProjectileCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ProjectileCollisionBox"));
	SetRootComponent(ProjectileCollisionBox);
	ProjectileCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	ProjectileCollisionBox->OnComponentHit.AddUniqueDynamic(this, &ThisClass::OnProjectileHit);
	ProjectileCollisionBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::OnProjectileBeginOverlap);

	ProjectileNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProjectileNiagaraComponent"));
	ProjectileNiagaraComponent->SetupAttachment(GetRootComponent());

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->InitialSpeed = 600.f;
	ProjectileMovementComponent->MaxSpeed = 600.f;
	ProjectileMovementComponent->ProjectileGravityScale = 0.f;
	ProjectileMovementComponent->Velocity = FVector(1.f, 0.f, 0.f);

	InitialLifeSpan = 4.f;
}

void ACombatProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (ProtectileDamagePolicy == EProtectileDamagePolicy::OnBeginOverlap)
	{
		ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}
	
	if (SoundAndFXSet.SpawnSound)
	{
		check(GetWorld());
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundAndFXSet.SpawnSound, GetActorLocation());
	}

	if (SoundAndFXSet.FlyingSound)
	{
		FlyingSoundComponent = UGameplayStatics::SpawnSoundAttached(
			SoundAndFXSet.FlyingSound,
			GetRootComponent(),
			FName(),
			FVector::ZeroVector,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}

	if (!SoundAndFXSet.MuzzleNiagaraSystem.IsNull())
	{
		UAssetManager::GetStreamableManager().RequestAsyncLoad(
			SoundAndFXSet.MuzzleNiagaraSystem.ToSoftObjectPath(),
			FStreamableDelegate::CreateUObject(this, &ACombatProjectileBase::OnAsyncLoadMuzzleNiagaraSystemFinished)
		);
	}

	if (!SoundAndFXSet.HitNiagaraSystem.IsNull())
	{
		UAssetManager::GetStreamableManager().RequestAsyncLoad(
			SoundAndFXSet.HitNiagaraSystem.ToSoftObjectPath(),
			FStreamableDelegate::CreateUObject(this, &ACombatProjectileBase::OnAsyncLoadHitNiagaraSystemFinished)
		);
	}
}

void ACombatProjectileBase::SetProjectileDamageEffectSpecHandle(const FGameplayEffectSpecHandle& InDamageEffectSpecHandle)
{
	ProjectileDamageEffectSpecHandle = InDamageEffectSpecHandle;
}

void ACombatProjectileBase::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// For Enemy
	PlayProjectileHitFX(Hit);

	// Check if hit APawn
	APawn* HitPawn = Cast<APawn>(OtherActor);

	if (!HitPawn || !UCombatFunctionLibrary::IsTargetPawnHostile(GetInstigator(), HitPawn))
	{
		FlyingSoundComponent->Stop();
		Destroy();
		return;
	}

	bool bIsValidBlock = false;
	const bool bIsPlayerBlocking = UCombatFunctionLibrary::NativeDoesActorHaveTag(OtherActor, CombatGameplayTags::Player_Status_Blocking);
	if (bIsPlayerBlocking)
	{
		bIsValidBlock = UCombatFunctionLibrary::IsValidBlock(this, OtherActor);
	}

	FGameplayEventData GameplayEventData;
	GameplayEventData.Instigator = this;
	GameplayEventData.Target = HitPawn;

	if (bIsValidBlock)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			HitPawn,
			CombatGameplayTags::Player_Event_SuccessfulBlock,
			GameplayEventData
		);
	}
	else
	{
		HandleApplyProjectileDamage(HitPawn, GameplayEventData);
	}

	FlyingSoundComponent->Stop();
	Destroy();
}

void ACombatProjectileBase::PlayProjectileHitFX(const FHitResult& Hit)
{
	if (SoundAndFXSet.ImpactSound)
	{
		check(GetWorld());
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundAndFXSet.ImpactSound, Hit.Location);
	}

	if (SoundAndFXSet.HitNiagaraSystem.IsValid())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			SoundAndFXSet.HitNiagaraSystem.Get(),
			GetActorLocation()
		);
	}
}

void ACombatProjectileBase::OnAsyncLoadMuzzleNiagaraSystemFinished()
{
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		SoundAndFXSet.MuzzleNiagaraSystem.Get(),
		GetActorLocation(),
		UKismetMathLibrary::Conv_VectorToRotator(GetActorForwardVector())
	);
	//Debug::Print(TEXT("Muzzle"));
}

void ACombatProjectileBase::OnAsyncLoadHitNiagaraSystemFinished()
{
	//Debug::Print(TEXT("Hit"));
}

void ACombatProjectileBase::OnProjectileBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// For Hero
	if (OverlappedActors.Contains(OtherActor))
	{
		return;
	}

	OverlappedActors.AddUnique(OtherActor);

	if (APawn* HitPawn = Cast<APawn>(OtherActor))
	{
		if (UCombatFunctionLibrary::IsTargetPawnHostile(GetInstigator(), HitPawn))
		{
			FGameplayEventData GameplayEventData;
			GameplayEventData.Instigator = GetInstigator();
			GameplayEventData.Target = OtherActor;

			HandleApplyProjectileDamage(HitPawn, GameplayEventData);
		}
	}
}

void ACombatProjectileBase::HandleApplyProjectileDamage(APawn* InHitPawn, const FGameplayEventData& InPayload)
{
	checkf(ProjectileDamageEffectSpecHandle.IsValid(), TEXT("Forgot to assign a valid spec handle to projectile %s"), *GetActorNameOrLabel());
	const bool bWasApplied = UCombatFunctionLibrary::ApplyGameplayEffectSpecHandleToTargetActor(
		GetInstigator(),
		InHitPawn, 
		ProjectileDamageEffectSpecHandle
	);

	if (bWasApplied)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			InHitPawn,
			CombatGameplayTags::Shared_Event_HitReact,
			InPayload
		);
	}
}
