#include "BondWeaponBase.h"

#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

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

	UWorld* World = GetWorld();

	if (World && FireInterval > 0.0f)
	{
		World->GetTimerManager().SetTimer(
			FireTimer,
			this,
			&ABondWeaponBase::FireOnce,
			FireInterval,
			true
		);
	}
}

void ABondWeaponBase::StopFire()
{
	bIsFiring = false;

	UWorld* World = GetWorld();

	if (World)
	{
		World->GetTimerManager().ClearTimer(FireTimer);
	}
}

void ABondWeaponBase::StartReload()
{
	if (!CanReload())
	{
		return;
	}

	StopFire();

	bIsReloading = true;
	OnReloadStarted.Broadcast();

	UWorld* World = GetWorld();

	if (World && ReloadDuration > 0.0f)
	{
		World->GetTimerManager().SetTimer(
			ReloadTimer,
			this,
			&ABondWeaponBase::CompleteReload,
			ReloadDuration,
			false
		);
		return;
	}

	CompleteReload();
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

	UWorld* World = GetWorld();

	if (World)
	{
		World->GetTimerManager().ClearTimer(ReloadTimer);
	}

	bIsReloading = false;

	BroadcastAmmoChanged();
	OnReloadFinished.Broadcast();
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

	const FVector TraceStart = GetTraceStart();
	const FVector TraceDirection = GetTraceDirection();
	const FVector TraceEnd = TraceStart + (TraceDirection * TraceRange);

	FHitResult Hit;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BondWeaponTrace), true, this);
	QueryParams.AddIgnoredActor(GetOwner());

	const bool bHit =
		World &&
		World->LineTraceSingleByChannel(
			Hit,
			TraceStart,
			TraceEnd,
			TraceChannel,
			QueryParams
		);

	if (bDrawDebugTrace && World)
	{
		const FColor TraceColor = bHit ? FColor::Red : FColor::Green;
		const FVector DebugEnd = bHit ? Hit.ImpactPoint : TraceEnd;

		DrawDebugLine(
			World,
			TraceStart,
			DebugEnd,
			TraceColor,
			false,
			2.0f,
			0,
			1.0f
		);
	}

	if (bHit && Hit.GetActor())
	{
		AController* InstigatorController = nullptr;
		APawn* OwnerPawn = Cast<APawn>(GetOwner());

		if (OwnerPawn)
		{
			InstigatorController = OwnerPawn->GetController();
		}

		UGameplayStatics::ApplyPointDamage(
			Hit.GetActor(),
			Damage,
			TraceDirection,
			Hit,
			InstigatorController,
			this,
			nullptr
		);
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("%s fired. Ammo: %d / %d"),
		*GetName(),
		MagazineAmmo,
		ReserveAmmo
	);
}

FVector ABondWeaponBase::GetTraceStart() const
{
	if (
		WeaponMesh &&
		MuzzleSocketName != NAME_None &&
		WeaponMesh->DoesSocketExist(MuzzleSocketName)
		)
	{
		return WeaponMesh->GetSocketLocation(MuzzleSocketName);
	}

	if (MuzzlePoint)
	{
		return MuzzlePoint->GetComponentLocation();
	}

	return GetActorLocation();
}

FVector ABondWeaponBase::GetTraceDirection() const
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());

	if (OwnerPawn && OwnerPawn->GetController())
	{
		FVector ViewLocation;
		FRotator ViewRotation;

		OwnerPawn->GetController()->GetPlayerViewPoint(
			ViewLocation,
			ViewRotation
		);

		return ViewRotation.Vector();
	}

	if (MuzzlePoint)
	{
		return MuzzlePoint->GetForwardVector();
	}

	return GetActorForwardVector();
}

void ABondWeaponBase::BroadcastAmmoChanged()
{
	OnAmmoChanged.Broadcast(
		MagazineAmmo,
		ReserveAmmo
	);
}
