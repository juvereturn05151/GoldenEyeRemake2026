#include "SWATWeaponComponent.h"

#include "../Characters/SWATEnemyCharacter.h"
#include "../Weapons/SWATProjectile.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
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

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[SWAT Weapon Init] ProjectileClass=%s WeaponMesh=%s MuzzleSocketName=%s HasMuzzle=%s CurrentMagazineAmmo=%d CurrentReserveAmmo=%d"),
		*GetNameSafe(ProjectileClass),
		*GetNameSafe(WeaponMesh),
		*MuzzleSocketName.ToString(),
		WeaponMesh && MuzzleSocketName != NAME_None && WeaponMesh->DoesSocketExist(MuzzleSocketName)
			? TEXT("true")
			: TEXT("false"),
		CurrentMagazineAmmo,
		CurrentReserveAmmo
	);

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
	SetWeaponSceneComponent(InWeaponMesh);
}

void USWATWeaponComponent::SetWeaponSceneComponent(USceneComponent* InWeaponMesh)
{
	WeaponMesh = InWeaponMesh;
}

void USWATWeaponComponent::SetStaticWeaponMesh(UStaticMeshComponent* InWeaponMesh)
{
	SetWeaponSceneComponent(InWeaponMesh);
}

void USWATWeaponComponent::EnableDebugDrawForNextShot()
{
	bDrawDebugForNextShot = true;
}

bool USWATWeaponComponent::FireProjectileAt(AActor* Target)
{
	AActor* OwnerActor = GetOwner();
	const bool bHasMuzzle =
		WeaponMesh &&
		MuzzleSocketName != NAME_None &&
		WeaponMesh->DoesSocketExist(MuzzleSocketName);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[SWAT Weapon] FireProjectileAt entered Owner=%s Target=%s ProjectileClass=%s WeaponMesh=%s MuzzleSocket=%s HasMuzzle=%s Ammo=%d Reloading=%s"),
		*GetNameSafe(OwnerActor),
		*GetNameSafe(Target),
		*GetNameSafe(ProjectileClass),
		*GetNameSafe(WeaponMesh),
		*MuzzleSocketName.ToString(),
		bHasMuzzle ? TEXT("true") : TEXT("false"),
		CurrentMagazineAmmo,
		bIsReloading ? TEXT("true") : TEXT("false")
	);

	if (!OwnerActor)
	{
		bDrawDebugForNextShot = false;
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Fire Failed] Owner invalid"));
		return false;
	}

	if (!Target)
	{
		bDrawDebugForNextShot = false;
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Fire Failed] Target invalid"));
		return false;
	}

	if (!ProjectileClass)
	{
		bDrawDebugForNextShot = false;
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Weapon] ProjectileClass is not assigned"));
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Fire Failed] ProjectileClass not assigned"));
		return false;
	}

	if (!WeaponMesh)
	{
		bDrawDebugForNextShot = false;
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Fire Failed] Weapon mesh not assigned"));
		return false;
	}

	if (!bHasMuzzle)
	{
		bDrawDebugForNextShot = false;
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[SWAT Fire Failed] Muzzle socket missing: %s"),
			*MuzzleSocketName.ToString()
		);
		return false;
	}

	const ASWATEnemyCharacter* SWATOwner =
		Cast<ASWATEnemyCharacter>(OwnerActor);

	if (!SWATOwner)
	{
		bDrawDebugForNextShot = false;
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Fire Failed] Owner invalid"));
		return false;
	}

	if (SWATOwner->IsDead())
	{
		bDrawDebugForNextShot = false;
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Fire Failed] Owner is dead"));
		return false;
	}

	if (SWATOwner->IsHitReacting())
	{
		bDrawDebugForNextShot = false;
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Fire Failed] Owner is hit reacting"));
		return false;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		bDrawDebugForNextShot = false;
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Fire Failed] World invalid"));
		return false;
	}

	const FTransform MuzzleTransform =
		WeaponMesh->GetSocketTransform(MuzzleSocketName, RTS_World);
	const FVector MuzzleLocation = MuzzleTransform.GetLocation();
	const FVector TargetLocation =
		Target->GetActorLocation() + FVector(0.0f, 0.0f, TargetAimHeightOffset);
	const FVector AimVector = TargetLocation - MuzzleLocation;

	if (AimVector.IsNearlyZero())
	{
		bDrawDebugForNextShot = false;
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Fire Failed] Aim direction invalid"));
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
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ASWATProjectile* SpawnedProjectile = World->SpawnActor<ASWATProjectile>(
		ProjectileClass,
		MuzzleLocation,
		SpawnRotation,
		SpawnParameters
	);

	if (!SpawnedProjectile)
	{
		bDrawDebugForNextShot = false;
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Weapon] SpawnActor failed"));
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Fire Failed] Projectile spawn returned null"));
		return false;
	}

	if (USphereComponent* ProjectileCollision = SpawnedProjectile->FindComponentByClass<USphereComponent>())
	{
		ProjectileCollision->IgnoreActorWhenMoving(OwnerActor, true);

		if (APawn* OwnerPawn = Cast<APawn>(OwnerActor))
		{
			ProjectileCollision->IgnoreActorWhenMoving(OwnerPawn, true);
		}

		if (UPrimitiveComponent* WeaponPrimitive = Cast<UPrimitiveComponent>(WeaponMesh))
		{
			ProjectileCollision->IgnoreComponentWhenMoving(WeaponPrimitive, true);
		}
	}

	if (bDrawDebugForNextShot)
	{
		DrawDebugSphere(World, MuzzleLocation, 12.0f, 12, FColor::Red, false, 2.0f);
		DrawDebugDirectionalArrow(
			World,
			MuzzleLocation,
			MuzzleLocation + FireDirection * 140.0f,
			24.0f,
			FColor::Yellow,
			false,
			2.0f,
			0,
			2.0f
		);
	}
	bDrawDebugForNextShot = false;

	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			FireSound,
			MuzzleLocation
		);
	}

	OnProjectileFired.Broadcast(SpawnedProjectile);

	const UProjectileMovementComponent* SpawnedMovement =
		SpawnedProjectile->FindComponentByClass<UProjectileMovementComponent>();
	const USphereComponent* SpawnedCollision =
		SpawnedProjectile->FindComponentByClass<USphereComponent>();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[SWAT Weapon] Projectile spawned: %s Speed=%.2f MaxSpeed=%.2f CollisionComponent=%s Hidden=%s LifeSpan=%.2f Direction=%s"),
		*GetNameSafe(SpawnedProjectile),
		SpawnedMovement ? SpawnedMovement->InitialSpeed : 0.0f,
		SpawnedMovement ? SpawnedMovement->MaxSpeed : 0.0f,
		*GetNameSafe(SpawnedCollision),
		SpawnedProjectile->IsHidden() ? TEXT("true") : TEXT("false"),
		SpawnedProjectile->GetLifeSpan(),
		*FireDirection.ToCompactString()
	);

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[SWAT Fire Success] Projectile=%s MuzzleLocation=%s Target=%s MagazineAmmo=%d"),
		*GetNameSafe(SpawnedProjectile),
		*MuzzleLocation.ToCompactString(),
		*GetNameSafe(Target),
		CurrentMagazineAmmo
	);

	return true;
}

bool USWATWeaponComponent::CanFire() const
{
	return CanOwnerUseWeapon();
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
