#include "BondWeaponBase.h"

#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

ABondWeaponBase::ABondWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	SetRootComponent(Root);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));

	WeaponMesh->SetupAttachment(Root);

	MuzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint"));

	MuzzlePoint->SetupAttachment(WeaponMesh);
}

void ABondWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	MagazineAmmo = FMath::Clamp(MagazineAmmo, 0, MagazineCapacity);

	ReserveAmmo = FMath::Max(0, StartingReserveAmmo);

	BroadcastAmmoChanged();
}

void ABondWeaponBase::StartFire()
{
	bIsFiring = true;

	FireOnce();
}

void ABondWeaponBase::StopFire()
{
	bIsFiring = false;
}

void ABondWeaponBase::StartReload()
{
	if (!CanReload())
	{
		return;
	}

	bIsReloading = true;
}

void ABondWeaponBase::CompleteReload()
{
	if (!bIsReloading)
	{
		return;
	}

	const int32 MissingAmmo = MagazineCapacity - MagazineAmmo;

	const int32 AmmoToLoad = FMath::Min(MissingAmmo, ReserveAmmo);

	MagazineAmmo += AmmoToLoad;
	ReserveAmmo -= AmmoToLoad;

	bIsReloading = false;

	BroadcastAmmoChanged();
}

bool ABondWeaponBase::CanFire() const
{
	const UWorld* World = GetWorld();

	const bool bFireIntervalElapsed = !World ||World->GetTimeSeconds() - LastFireTime >= FireInterval;

	return !bIsReloading && MagazineAmmo > 0 && bFireIntervalElapsed;
}

bool ABondWeaponBase::CanReload() const
{
	return !bIsReloading && MagazineAmmo < MagazineCapacity && ReserveAmmo > 0;
}

USceneComponent* ABondWeaponBase::GetMuzzlePoint() const
{
	return MuzzlePoint;
}

FName ABondWeaponBase::GetMuzzleSocketName() const
{
	return MuzzleSocketName;
}

void ABondWeaponBase::FireOnce()
{
	if (!CanFire())
	{
		return;
	}

	UWorld* World = GetWorld();

	if (World)
	{
		LastFireTime = World->GetTimeSeconds();
	}

	--MagazineAmmo;

	BroadcastAmmoChanged();

	UE_LOG(
		LogTemp,
		Log,
		TEXT("%s fired. Ammo: %d / %d"),
		*GetName(),
		MagazineAmmo,
		ReserveAmmo
	);
}

void ABondWeaponBase::BroadcastAmmoChanged()
{
	OnAmmoChanged.Broadcast(
		MagazineAmmo,
		ReserveAmmo
	);
}
