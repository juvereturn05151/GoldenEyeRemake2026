#include "AmmoRefillComponent.h"

#include "../Characters/JamesBondCharacter.h"
#include "../Components/BondWeaponComponent.h"
#include "../Weapons/BondWeaponBase.h"
#include "Components/PrimitiveComponent.h"

UAmmoRefillComponent::UAmmoRefillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAmmoRefillComponent::BeginPlay()
{
	Super::BeginPlay();

	UPrimitiveComponent* ResolvedOverlapComponent = ResolveOverlapComponent();

	if (!ResolvedOverlapComponent)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[Ammo Refill] Owner=%s has no primitive overlap component"),
			*GetNameSafe(GetOwner())
		);
		return;
	}

	OverlapComponent = ResolvedOverlapComponent;
	OverlapComponent->OnComponentBeginOverlap.AddUniqueDynamic(
		this,
		&UAmmoRefillComponent::HandleComponentBeginOverlap
	);
}

void UAmmoRefillComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (OverlapComponent)
	{
		OverlapComponent->OnComponentBeginOverlap.RemoveDynamic(
			this,
			&UAmmoRefillComponent::HandleComponentBeginOverlap
		);
	}

	Super::EndPlay(EndPlayReason);
}

bool UAmmoRefillComponent::TryRefillAmmo(AActor* PickupActor)
{
	if (bHasBeenPickedUp || !PickupActor || AmmoAmount <= 0)
	{
		return false;
	}

	AJamesBondCharacter* BondCharacter = Cast<AJamesBondCharacter>(PickupActor);

	if (!BondCharacter)
	{
		return false;
	}

	UBondWeaponComponent* WeaponComponent = BondCharacter->GetWeaponComponent();
	ABondWeaponBase* EquippedWeapon =
		WeaponComponent ? WeaponComponent->GetEquippedWeapon() : nullptr;

	if (!EquippedWeapon)
	{
		return false;
	}

	const int32 AddedAmmo = EquippedWeapon->AddReserveAmmo(AmmoAmount);

	if (AddedAmmo <= 0)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[Ammo Refill] Bond=%s Weapon=%s pickup ignored because reserve ammo is full"),
			*GetNameSafe(BondCharacter),
			*GetNameSafe(EquippedWeapon)
		);
		return false;
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[Ammo Refill] Bond=%s Weapon=%s AddedAmmo=%d RequestedAmmo=%d"),
		*GetNameSafe(BondCharacter),
		*GetNameSafe(EquippedWeapon),
		AddedAmmo,
		AmmoAmount
	);

	CompletePickup();
	return true;
}

void UAmmoRefillComponent::HandleComponentBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	TryRefillAmmo(OtherActor);
}

UPrimitiveComponent* UAmmoRefillComponent::ResolveOverlapComponent() const
{
	if (OverlapComponent)
	{
		return OverlapComponent;
	}

	AActor* OwnerActor = GetOwner();

	if (!OwnerActor)
	{
		return nullptr;
	}

	if (UPrimitiveComponent* RootPrimitive =
		Cast<UPrimitiveComponent>(OwnerActor->GetRootComponent()))
	{
		return RootPrimitive;
	}

	return OwnerActor->FindComponentByClass<UPrimitiveComponent>();
}

void UAmmoRefillComponent::CompletePickup()
{
	bHasBeenPickedUp = true;

	if (OverlapComponent && bDisableAfterPickup)
	{
		OverlapComponent->SetGenerateOverlapEvents(false);
		OverlapComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (AActor* OwnerActor = GetOwner(); OwnerActor && bDestroyOwnerAfterPickup)
	{
		OwnerActor->Destroy();
	}
}
