#include "SWATWeaponComponent.h"

#include "../Characters/SWATEnemyCharacter.h"
#include "../Weapons/SWATProjectile.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

USWATWeaponComponent::USWATWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USWATWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentMagazineAmmo = FMath::Max(0, MagazineCapacity);
	CurrentReserveAmmo = FMath::Max(0, StartingReserveAmmo);

	if (ASWATEnemyCharacter* SWATOwner = Cast<ASWATEnemyCharacter>(GetOwner()))
	{
		SWATOwner->OnSWATStateChanged.AddDynamic(
			this,
			&USWATWeaponComponent::HandleOwnerStateChanged
		);
	}

	BroadcastAmmoChanged();
}

void USWATWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ASWATEnemyCharacter* SWATOwner = Cast<ASWATEnemyCharacter>(GetOwner()))
	{
		SWATOwner->OnSWATStateChanged.RemoveDynamic(
			this,
			&USWATWeaponComponent::HandleOwnerStateChanged
		);
	}

	CancelReload();

	Super::EndPlay(EndPlayReason);
}

void USWATWeaponComponent::SetWeaponMesh(USkeletalMeshComponent* InWeaponMesh)
{
	WeaponMesh = InWeaponMesh;
}

bool USWATWeaponComponent::FireProjectileAt(AActor* Target)
{
	AActor* OwnerActor = GetOwner();

	if (
		!OwnerActor ||
		!Target ||
		!ProjectileClass ||
		!WeaponMesh ||
		MuzzleSocketName == NAME_None ||
		!WeaponMesh->DoesSocketExist(MuzzleSocketName) ||
		!CanFire()
		)
	{
		return false;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		return false;
	}

	const FVector MuzzleLocation =
		WeaponMesh->GetSocketLocation(MuzzleSocketName);
	const FVector TargetLocation =
		Target->GetActorLocation() + FVector(0.0f, 0.0f, TargetAimHeightOffset);
	const FVector AimVector = TargetLocation - MuzzleLocation;

	if (AimVector.IsNearlyZero())
	{
		return false;
	}

	const FVector AimDirection = AimVector.GetSafeNormal();
	const float SpreadRadians = FMath::DegreesToRadians(BaseSpreadDegrees);
	const FVector FireDirection =
		BaseSpreadDegrees > 0.0f
			? FMath::VRandCone(AimDirection, SpreadRadians)
			: AimDirection;
	const FRotator SpawnRotation = FireDirection.Rotation();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = OwnerActor;
	SpawnParameters.Instigator = Cast<APawn>(OwnerActor);
	SpawnParameters.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ASWATProjectile* SpawnedProjectile = World->SpawnActor<ASWATProjectile>(
		ProjectileClass,
		MuzzleLocation,
		SpawnRotation,
		SpawnParameters
	);

	if (!SpawnedProjectile)
	{
		return false;
	}

	--CurrentMagazineAmmo;
	BroadcastAmmoChanged();
	OnProjectileFired.Broadcast(SpawnedProjectile);

	return true;
}

bool USWATWeaponComponent::CanFire() const
{
	return
		CanOwnerUseWeapon() &&
		!bIsReloading &&
		CurrentMagazineAmmo > 0;
}

bool USWATWeaponComponent::StartReload()
{
	if (
		!CanOwnerUseWeapon() ||
		bIsReloading ||
		CurrentMagazineAmmo >= MagazineCapacity ||
		CurrentReserveAmmo <= 0
		)
	{
		return false;
	}

	SetReloadingState(true);
	OnReloadStarted.Broadcast();

	UWorld* World = GetWorld();

	if (World && ReloadDuration > 0.0f)
	{
		World->GetTimerManager().SetTimer(
			ReloadTimerHandle,
			this,
			&USWATWeaponComponent::CompleteReload,
			ReloadDuration,
			false
		);
		return true;
	}

	CompleteReload();
	return true;
}

void USWATWeaponComponent::CompleteReload()
{
	if (!bIsReloading)
	{
		return;
	}

	if (!CanOwnerUseWeapon())
	{
		CancelReload();
		return;
	}

	const int32 MissingAmmo = FMath::Max(0, MagazineCapacity - CurrentMagazineAmmo);
	const int32 AmmoToLoad = FMath::Min(MissingAmmo, CurrentReserveAmmo);

	CurrentMagazineAmmo += AmmoToLoad;
	CurrentReserveAmmo = FMath::Max(0, CurrentReserveAmmo - AmmoToLoad);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReloadTimerHandle);
	}

	SetReloadingState(false);
	BroadcastAmmoChanged();
	OnReloadFinished.Broadcast();
}

void USWATWeaponComponent::CancelReload()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReloadTimerHandle);
	}

	if (!bIsReloading)
	{
		return;
	}

	SetReloadingState(false);
}

void USWATWeaponComponent::StopWeapon()
{
	CancelReload();

	if (ASWATEnemyCharacter* SWATOwner = Cast<ASWATEnemyCharacter>(GetOwner()))
	{
		SWATOwner->SetFiring(false);
	}
}

bool USWATWeaponComponent::NeedsReload() const
{
	return CurrentMagazineAmmo <= 0 && CurrentReserveAmmo > 0;
}

bool USWATWeaponComponent::IsReloading() const
{
	return bIsReloading;
}

int32 USWATWeaponComponent::GetMagazineAmmo() const
{
	return CurrentMagazineAmmo;
}

int32 USWATWeaponComponent::GetReserveAmmo() const
{
	return CurrentReserveAmmo;
}

void USWATWeaponComponent::HandleOwnerStateChanged()
{
	const ASWATEnemyCharacter* SWATOwner =
		Cast<ASWATEnemyCharacter>(GetOwner());

	if (SWATOwner && SWATOwner->IsDead())
	{
		StopWeapon();
	}
}

bool USWATWeaponComponent::CanOwnerUseWeapon() const
{
	const ASWATEnemyCharacter* SWATOwner =
		Cast<ASWATEnemyCharacter>(GetOwner());

	return
		SWATOwner &&
		!SWATOwner->IsDead() &&
		!SWATOwner->IsHitReacting();
}

void USWATWeaponComponent::BroadcastAmmoChanged()
{
	OnAmmoChanged.Broadcast(
		CurrentMagazineAmmo,
		CurrentReserveAmmo
	);
}

void USWATWeaponComponent::SetReloadingState(bool bNewIsReloading)
{
	if (bIsReloading == bNewIsReloading)
	{
		return;
	}

	bIsReloading = bNewIsReloading;

	if (ASWATEnemyCharacter* SWATOwner = Cast<ASWATEnemyCharacter>(GetOwner()))
	{
		SWATOwner->SetReloading(bIsReloading);
	}
}
