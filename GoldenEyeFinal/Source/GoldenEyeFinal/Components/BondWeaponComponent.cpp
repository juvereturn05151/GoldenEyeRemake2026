#include "BondWeaponComponent.h"

#include "../Characters/JamesBondCharacter.h"
#include "../Weapons/BondWeaponBase.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

UBondWeaponComponent::UBondWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBondWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	SpawnDefaultWeapon();
}

void UBondWeaponComponent::SpawnDefaultWeapon()
{
	if (EquippedWeapon || !DefaultWeaponClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	AActor* OwnerActor = GetOwner();

	if (!World || !OwnerActor)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = OwnerActor;
	SpawnParameters.Instigator = Cast<APawn>(OwnerActor);
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ABondWeaponBase* SpawnedWeapon =
		World->SpawnActor<ABondWeaponBase>(
			DefaultWeaponClass,
			FTransform::Identity,
			SpawnParameters
		);

	EquipWeapon(SpawnedWeapon);
}

void UBondWeaponComponent::EquipWeapon(ABondWeaponBase* NewWeapon)
{
	if (!NewWeapon)
	{
		return;
	}

	AJamesBondCharacter* BondOwner = Cast<AJamesBondCharacter>(GetOwner());

	if (!BondOwner)
	{
		return;
	}

	USceneComponent* WeaponRoot = BondOwner->GetWeaponRoot();

	if (!WeaponRoot)
	{
		return;
	}

	NewWeapon->SetOwner(BondOwner);
	NewWeapon->AttachToComponent(
		WeaponRoot,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale
	);

	EquippedWeapon = NewWeapon;

	OnWeaponEquipped.Broadcast(
		EquippedWeapon
	);
}

void UBondWeaponComponent::StartFire()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->StartFire();
	}
}

void UBondWeaponComponent::StopFire()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->StopFire();
	}
}

void UBondWeaponComponent::Reload()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->StartReload();
	}
}

ABondWeaponBase* UBondWeaponComponent::GetEquippedWeapon() const
{
	return EquippedWeapon;
}
