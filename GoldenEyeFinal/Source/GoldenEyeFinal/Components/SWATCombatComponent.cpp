#include "SWATCombatComponent.h"

#include "../Characters/SWATEnemyCharacter.h"
#include "SWATWeaponComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

USWATCombatComponent::USWATCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USWATCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	BindOwnerAndWeapon();
}

void USWATCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopCombatBurst();
	UnbindOwnerAndWeapon();

	Super::EndPlay(EndPlayReason);
}

bool USWATCombatComponent::StartCombatBurst(AActor* Target)
{
	ASWATEnemyCharacter* SWATOwner = GetSWATOwner();
	USWATWeaponComponent* WeaponComponent = GetWeaponComponent();

	if (
		!SWATOwner ||
		!Target ||
		!WeaponComponent ||
		SWATOwner->IsDead() ||
		SWATOwner->IsHitReacting() ||
		!bHasLineOfSight ||
		WeaponComponent->IsReloading() ||
		WeaponComponent->GetMagazineAmmo() <= 0 ||
		bBurstActive ||
		bBurstOnCooldown
		)
	{
		return false;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		return false;
	}

	CurrentTarget = Target;
	RemainingBurstShots = 0;
	bBurstActive = true;

	SWATOwner->SetFiring(true);
	OnBurstStarted.Broadcast();

	World->GetTimerManager().ClearTimer(AimDelayTimerHandle);

	if (AimPreparationTime > 0.0f)
	{
		World->GetTimerManager().SetTimer(
			AimDelayTimerHandle,
			this,
			&USWATCombatComponent::BeginBurstShots,
			AimPreparationTime,
			false
		);
	}
	else
	{
		BeginBurstShots();
	}

	return true;
}

void USWATCombatComponent::StopCombatBurst()
{
	if (!bBurstActive)
	{
		ClearBurstTimers();
		CurrentTarget = nullptr;
		RemainingBurstShots = 0;
		return;
	}

	FinishBurst(false);
}

bool USWATCombatComponent::CanStartBurst() const
{
	const ASWATEnemyCharacter* SWATOwner =
		Cast<ASWATEnemyCharacter>(GetOwner());
	const USWATWeaponComponent* WeaponComponent =
		SWATOwner ? SWATOwner->GetWeaponComponent() : nullptr;

	return
		SWATOwner &&
		WeaponComponent &&
		!SWATOwner->IsDead() &&
		!SWATOwner->IsHitReacting() &&
		bHasLineOfSight &&
		!WeaponComponent->IsReloading() &&
		WeaponComponent->GetMagazineAmmo() > 0 &&
		!bBurstActive &&
		!bBurstOnCooldown;
}

bool USWATCombatComponent::IsBurstActive() const
{
	return bBurstActive;
}

bool USWATCombatComponent::IsBurstOnCooldown() const
{
	return bBurstOnCooldown;
}

void USWATCombatComponent::SetHasLineOfSight(bool bNewHasLineOfSight)
{
	if (bHasLineOfSight == bNewHasLineOfSight)
	{
		return;
	}

	bHasLineOfSight = bNewHasLineOfSight;

	if (!bHasLineOfSight)
	{
		StopCombatBurst();
	}
}

void USWATCombatComponent::HandleOwnerStateChanged()
{
	const ASWATEnemyCharacter* SWATOwner = Cast<ASWATEnemyCharacter>(GetOwner());

	if (!SWATOwner || SWATOwner->IsDead() || SWATOwner->IsHitReacting())
	{
		StopCombatBurst();
	}
}

void USWATCombatComponent::HandleWeaponAmmoChanged(int32 MagazineAmmo, int32 ReserveAmmo)
{
	if (MagazineAmmo <= 0)
	{
		StopCombatBurst();
	}
}

void USWATCombatComponent::HandleWeaponReloadStarted()
{
	StopCombatBurst();
}

void USWATCombatComponent::BeginBurstShots()
{
	if (!ValidateBurstContinuation())
	{
		FinishBurst(false);
		return;
	}

	const int32 MinShots = FMath::Max(1, MinimumBurstShots);
	const int32 MaxShots = FMath::Max(MinShots, MaximumBurstShots);

	RemainingBurstShots = FMath::RandRange(MinShots, MaxShots);
	FireNextBurstShot();
}

void USWATCombatComponent::FireNextBurstShot()
{
	if (!ValidateBurstContinuation())
	{
		FinishBurst(false);
		return;
	}

	USWATWeaponComponent* WeaponComponent = GetWeaponComponent();

	if (!WeaponComponent || !WeaponComponent->FireProjectileAt(CurrentTarget))
	{
		FinishBurst(false);
		return;
	}

	--RemainingBurstShots;
	OnBurstShot.Broadcast();

	if (RemainingBurstShots <= 0)
	{
		FinishBurst(true);
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		FinishBurst(false);
		return;
	}

	World->GetTimerManager().ClearTimer(ShotIntervalTimerHandle);

	if (TimeBetweenShots > 0.0f)
	{
		World->GetTimerManager().SetTimer(
			ShotIntervalTimerHandle,
			this,
			&USWATCombatComponent::FireNextBurstShot,
			TimeBetweenShots,
			false
		);
	}
	else
	{
		FireNextBurstShot();
	}
}

void USWATCombatComponent::FinishBurst(bool bStartCooldown)
{
	ClearBurstTimers();
	CurrentTarget = nullptr;
	RemainingBurstShots = 0;

	const bool bWasBurstActive = bBurstActive;
	bBurstActive = false;

	if (ASWATEnemyCharacter* SWATOwner = GetSWATOwner())
	{
		SWATOwner->SetFiring(false);
	}

	if (bStartCooldown && BurstCooldown > 0.0f)
	{
		if (UWorld* World = GetWorld())
		{
			bBurstOnCooldown = true;
			World->GetTimerManager().SetTimer(
				BurstCooldownTimerHandle,
				this,
				&USWATCombatComponent::FinishBurstCooldown,
				BurstCooldown,
				false
			);
		}
	}

	if (bWasBurstActive)
	{
		OnBurstFinished.Broadcast();
	}
}

void USWATCombatComponent::FinishBurstCooldown()
{
	bBurstOnCooldown = false;
}

bool USWATCombatComponent::ValidateBurstContinuation() const
{
	const ASWATEnemyCharacter* SWATOwner = Cast<ASWATEnemyCharacter>(GetOwner());
	const USWATWeaponComponent* WeaponComponent = SWATOwner ? SWATOwner->GetWeaponComponent() : nullptr;

	return
		SWATOwner &&
		CurrentTarget &&
		WeaponComponent &&
		!SWATOwner->IsDead() &&
		!SWATOwner->IsHitReacting() &&
		bHasLineOfSight &&
		!WeaponComponent->IsReloading() &&
		WeaponComponent->GetMagazineAmmo() > 0;
}

ASWATEnemyCharacter* USWATCombatComponent::GetSWATOwner() const
{
	return Cast<ASWATEnemyCharacter>(GetOwner());
}

USWATWeaponComponent* USWATCombatComponent::GetWeaponComponent() const
{
	const ASWATEnemyCharacter* SWATOwner = Cast<ASWATEnemyCharacter>(GetOwner());

	return SWATOwner ? SWATOwner->GetWeaponComponent() : nullptr;
}

void USWATCombatComponent::ClearBurstTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AimDelayTimerHandle);
		World->GetTimerManager().ClearTimer(ShotIntervalTimerHandle);
	}
}

void USWATCombatComponent::BindOwnerAndWeapon()
{
	if (ASWATEnemyCharacter* SWATOwner = GetSWATOwner())
	{
		SWATOwner->OnSWATStateChanged.AddUniqueDynamic(
			this,
			&USWATCombatComponent::HandleOwnerStateChanged
		);

		if (USWATWeaponComponent* WeaponComponent = SWATOwner->GetWeaponComponent())
		{
			WeaponComponent->OnAmmoChanged.AddUniqueDynamic(
				this,
				&USWATCombatComponent::HandleWeaponAmmoChanged
			);
			WeaponComponent->OnReloadStarted.AddUniqueDynamic(
				this,
				&USWATCombatComponent::HandleWeaponReloadStarted
			);
		}
	}
}

void USWATCombatComponent::UnbindOwnerAndWeapon()
{
	if (ASWATEnemyCharacter* SWATOwner = GetSWATOwner())
	{
		SWATOwner->OnSWATStateChanged.RemoveDynamic(
			this,
			&USWATCombatComponent::HandleOwnerStateChanged
		);

		if (USWATWeaponComponent* WeaponComponent = SWATOwner->GetWeaponComponent())
		{
			WeaponComponent->OnAmmoChanged.RemoveDynamic(
				this,
				&USWATCombatComponent::HandleWeaponAmmoChanged
			);
			WeaponComponent->OnReloadStarted.RemoveDynamic(
				this,
				&USWATCombatComponent::HandleWeaponReloadStarted
			);
		}
	}
}
