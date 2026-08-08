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

	if (!SWATOwner)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat] Start burst rejected: owner invalid"));
		return false;
	}

	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat] Start burst rejected: target invalid"));
		return false;
	}

	if (!WeaponComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat] Start burst rejected: weapon component invalid"));
		return false;
	}

	if (SWATOwner->IsDead())
	{
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat] Start burst rejected: owner is dead"));
		return false;
	}

	if (SWATOwner->IsHitReacting())
	{
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat] Start burst rejected: owner is hit reacting"));
		return false;
	}

	if (!bHasLineOfSight)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat] Start burst rejected: no line of sight"));
		return false;
	}

	if (bBurstActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat] Start burst rejected: burst already active"));
		return false;
	}

	if (bBurstOnCooldown)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat] Start burst rejected: burst cooldown active"));
		return false;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat] Start burst rejected: world invalid"));
		return false;
	}

	CurrentTarget = Target;
	const int32 MinShots = FMath::Max(1, MinimumBurstShots);
	const int32 MaxShots = FMath::Max(MinShots, MaximumBurstShots);

	RemainingBurstShots = FMath::RandRange(MinShots, MaxShots);
	bBurstActive = true;
	bWaitingForFireNotify = false;

	SWATOwner->SetFiring(true);
	OnBurstStarted.Broadcast();
	UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat] Burst started"));

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
		bWaitingForFireNotify = false;
		return;
	}

	FinishBurst(false);
}

bool USWATCombatComponent::CanStartBurst() const
{
	const ASWATEnemyCharacter* SWATOwner = Cast<ASWATEnemyCharacter>(GetOwner());
	const USWATWeaponComponent* WeaponComponent = SWATOwner ? SWATOwner->GetWeaponComponent() : nullptr;

	return
		SWATOwner &&
		WeaponComponent &&
		!SWATOwner->IsDead() &&
		!SWATOwner->IsHitReacting() &&
		bHasLineOfSight &&
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

bool USWATCombatComponent::IsWaitingForFireNotify() const
{
	return bWaitingForFireNotify;
}

AActor* USWATCombatComponent::GetCurrentTarget() const
{
	return CurrentTarget;
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

bool USWATCombatComponent::AcceptFireProjectileNotify()
{
	const ASWATEnemyCharacter* SWATOwner = GetSWATOwner();
	const USWATWeaponComponent* WeaponComponent = GetWeaponComponent();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[SWAT Combat] Fire notify received BurstActive=%s Waiting=%s Target=%s"),
		bBurstActive ? TEXT("true") : TEXT("false"),
		bWaitingForFireNotify ? TEXT("true") : TEXT("false"),
		*GetNameSafe(CurrentTarget)
	);

	if (!bBurstActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat] Fire notify ignored: burst is not active"));
		return false;
	}

	if (!bWaitingForFireNotify)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat] Fire notify ignored: no active request"));
		return false;
	}

	if (!CurrentTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat] Fire notify rejected: CurrentTarget invalid"));
		StopCombatBurst();
		return false;
	}

	if (!SWATOwner)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat] Fire notify rejected: owner invalid"));
		StopCombatBurst();
		return false;
	}

	if (SWATOwner->IsDead())
	{
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat] Fire notify rejected: owner is dead"));
		StopCombatBurst();
		return false;
	}

	if (SWATOwner->IsHitReacting())
	{
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat] Fire notify rejected: owner is hit reacting"));
		StopCombatBurst();
		return false;
	}

	if (!WeaponComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat] Fire notify rejected: weapon component invalid"));
		StopCombatBurst();
		return false;
	}

	if (!bHasLineOfSight)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat] Fire notify rejected: no line of sight"));
		StopCombatBurst();
		return false;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FireNotifyTimeoutTimerHandle);
	}

	bWaitingForFireNotify = false;
	UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat] Fire notify accepted"));
	return true;
}

void USWATCombatComponent::ConfirmFireProjectileFromAnimation()
{
	if (!AcceptFireProjectileNotify())
	{
		return;
	}

	USWATWeaponComponent* WeaponComponent = GetWeaponComponent();

	if (!WeaponComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat] Fire notify rejected: weapon component invalid"));
		FinishBurst(false);
		return;
	}

	WeaponComponent->EnableDebugDrawForNextShot();
	const bool bProjectileFired = WeaponComponent->FireProjectileAt(CurrentTarget);
	CompleteAnimatedShot(bProjectileFired);
}

void USWATCombatComponent::CompleteAnimatedShot(bool bProjectileFired)
{
	if (!bBurstActive)
	{
		return;
	}

	if (!bProjectileFired)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat] Projectile confirmation failed"));
		FinishBurst(false);
		return;
	}

	--RemainingBurstShots;
	OnBurstShot.Broadcast();
	UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat] Projectile confirmed"));

	if (RemainingBurstShots <= 0)
	{
		FinishBurst(true);
		return;
	}

	if (!ValidateBurstContinuation())
	{
		FinishBurst(false);
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		FinishBurst(false);
		return;
	}

	World->GetTimerManager().ClearTimer(NextShotDelayTimerHandle);

	if (TimeBetweenShots > 0.0f)
	{
		World->GetTimerManager().SetTimer(
			NextShotDelayTimerHandle,
			this,
			&USWATCombatComponent::RequestNextAnimatedShot,
			TimeBetweenShots,
			false
		);
	}
	else
	{
		RequestNextAnimatedShot();
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
}

void USWATCombatComponent::HandleWeaponReloadStarted()
{
}

void USWATCombatComponent::BeginBurstShots()
{
	if (!ValidateBurstContinuation())
	{
		FinishBurst(false);
		return;
	}

	RequestNextAnimatedShot();
}

void USWATCombatComponent::RequestNextAnimatedShot()
{
	if (!ValidateBurstContinuation())
	{
		FinishBurst(false);
		return;
	}

	ASWATEnemyCharacter* SWATOwner = GetSWATOwner();
	UWorld* World = GetWorld();

	if (!SWATOwner || !World)
	{
		FinishBurst(false);
		return;
	}

	bWaitingForFireNotify = true;
	SWATOwner->OnSWATFireAnimationRequested();
	UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat] Fire animation requested"));

	World->GetTimerManager().ClearTimer(FireNotifyTimeoutTimerHandle);

	if (FireNotifyTimeout > 0.0f)
	{
		World->GetTimerManager().SetTimer(
			FireNotifyTimeoutTimerHandle,
			this,
			&USWATCombatComponent::HandleFireNotifyTimeout,
			FireNotifyTimeout,
			false
		);
	}
}

void USWATCombatComponent::HandleFireNotifyTimeout()
{
	if (!bBurstActive || !bWaitingForFireNotify)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat] Fire notify timeout"));
	FinishBurst(false);
}

void USWATCombatComponent::FinishBurst(bool bStartCooldown)
{
	ClearBurstTimers();
	CurrentTarget = nullptr;
	RemainingBurstShots = 0;
	bWaitingForFireNotify = false;

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
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat] Burst finished"));
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
		bHasLineOfSight;
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
		World->GetTimerManager().ClearTimer(NextShotDelayTimerHandle);
		World->GetTimerManager().ClearTimer(FireNotifyTimeoutTimerHandle);
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
