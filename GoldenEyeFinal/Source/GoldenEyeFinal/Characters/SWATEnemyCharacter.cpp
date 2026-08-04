#include "SWATEnemyCharacter.h"

#include "../Components/NPCHealthComponent.h"
#include "../Components/SWATCombatComponent.h"
#include "../Components/SWATWeaponComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "TimerManager.h"

ASWATEnemyCharacter::ASWATEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	bUseControllerRotationYaw = true;

	HealthComponent = CreateDefaultSubobject<UNPCHealthComponent>(TEXT("HealthComponent"));
	WeaponComponent = CreateDefaultSubobject<USWATWeaponComponent>(TEXT("WeaponComponent"));
	CombatComponent = CreateDefaultSubobject<USWATCombatComponent>(TEXT("CombatComponent"));

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	if (MovementComponent)
	{
		MovementComponent->bOrientRotationToMovement = false;
		MovementComponent->bUseControllerDesiredRotation = false;
		MovementComponent->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	}
}

void ASWATEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	bUseControllerRotationYaw = true;

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->bOrientRotationToMovement = false;
		MovementComponent->bUseControllerDesiredRotation = false;
		MovementComponent->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	}

	if (HealthComponent)
	{
		HealthComponent->OnDamageTaken.AddDynamic(
			this,
			&ASWATEnemyCharacter::HandleDamageTaken
		);

		HealthComponent->OnDeath.AddDynamic(
			this,
			&ASWATEnemyCharacter::HandleDeath
		);
	}
}

UNPCHealthComponent* ASWATEnemyCharacter::GetHealthComponent() const
{
	return HealthComponent;
}

USWATWeaponComponent* ASWATEnemyCharacter::GetWeaponComponent() const
{
	return WeaponComponent;
}

USWATCombatComponent* ASWATEnemyCharacter::GetCombatComponent() const
{
	return CombatComponent;
}

bool ASWATEnemyCharacter::IsDead() const
{
	return bIsDead;
}

bool ASWATEnemyCharacter::IsHitReacting() const
{
	return bIsHitReacting;
}

bool ASWATEnemyCharacter::IsInCombat() const
{
	return bIsInCombat;
}

void ASWATEnemyCharacter::SetInCombat(bool bNewIsInCombat)
{
	const bool bNewValue = !bIsDead && bNewIsInCombat;

	if (bIsInCombat == bNewValue)
	{
		return;
	}

	bIsInCombat = bNewValue;
	BroadcastStateChanged();
}

void ASWATEnemyCharacter::SetReloading(bool bNewIsReloading)
{
	const bool bNewValue = !bIsDead && bNewIsReloading;

	if (bIsReloading == bNewValue)
	{
		return;
	}

	bIsReloading = bNewValue;
	BroadcastStateChanged();
}

void ASWATEnemyCharacter::SetHasLineOfSight(bool bNewHasLineOfSight)
{
	const bool bNewValue = !bIsDead && bNewHasLineOfSight;

	if (bHasLineOfSight == bNewValue)
	{
		return;
	}

	bHasLineOfSight = bNewValue;

	if (CombatComponent)
	{
		CombatComponent->SetHasLineOfSight(bHasLineOfSight);
	}

	BroadcastStateChanged();
}

void ASWATEnemyCharacter::SetFiring(bool bNewIsFiring)
{
	const bool bNewValue = !bIsDead && bNewIsFiring;

	if (bIsFiring == bNewValue)
	{
		return;
	}

	bIsFiring = bNewValue;
	BroadcastStateChanged();
}

void ASWATEnemyCharacter::ConfirmFireProjectileFromAnimation()
{
	UE_LOG(LogTemp, Warning, TEXT("[SWAT Character] ConfirmFireProjectileFromAnimation entered"));

	if (bIsDead)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Character] Fire notify rejected: owner is dead"));
		return;
	}

	if (bIsHitReacting)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Character] Fire notify rejected: owner is hit reacting"));
		return;
	}

	if (!CombatComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Character] Fire notify rejected: CombatComponent invalid"));
		return;
	}

	CombatComponent->ConfirmFireProjectileFromAnimation();
}

void ASWATEnemyCharacter::HandleDeath()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;

	UWorld* World = GetWorld();

	if (World)
	{
		World->GetTimerManager().ClearTimer(HitReactionMovementLockTimer);
		World->GetTimerManager().ClearTimer(HitReactionCooldownTimer);
	}

	bIsHitReacting = false;
	bIsHitReactionOnCooldown = false;

	StopMovementOnDeath();
	StopCombatOnDeath();
	BroadcastStateChanged();
	OnSWATDeath();
}

void ASWATEnemyCharacter::HandleDamageTaken(float DamageAmount)
{
	if (
		bIsDead ||
		DamageAmount <= 0.0f ||
		bIsHitReacting ||
		bIsHitReactionOnCooldown ||
		!HealthComponent ||
		HealthComponent->GetCurrentHealth() <= 0.0f
		)
	{
		return;
	}

	LockMovementForHitReaction();
	OnSWATHitReaction(DamageAmount);

	UWorld* World = GetWorld();

	if (World)
	{
		World->GetTimerManager().ClearTimer(HitReactionMovementLockTimer);

		if (HitReactionMovementLockDuration > 0.0f)
		{
			World->GetTimerManager().SetTimer(
				HitReactionMovementLockTimer,
				this,
				&ASWATEnemyCharacter::RestoreMovementAfterHitReaction,
				HitReactionMovementLockDuration,
				false
			);
		}
		else
		{
			RestoreMovementAfterHitReaction();
		}
	}

	if (HitReactionCooldown <= 0.0f)
	{
		return;
	}

	bIsHitReactionOnCooldown = true;

	if (World)
	{
		World->GetTimerManager().SetTimer(
			HitReactionCooldownTimer,
			this,
			&ASWATEnemyCharacter::EnableHitReaction,
			HitReactionCooldown,
			false
		);
	}
}

void ASWATEnemyCharacter::StopMovementOnDeath()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	if (MovementComponent)
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
	}

	AController* CurrentController = GetController();

	if (CurrentController)
	{
		CurrentController->StopMovement();
	}
}

void ASWATEnemyCharacter::StopCombatOnDeath()
{
	if (WeaponComponent)
	{
		WeaponComponent->StopWeapon();
	}

	if (CombatComponent)
	{
		CombatComponent->StopCombatBurst();
	}

	bIsInCombat = false;
	bIsReloading = false;
	bHasLineOfSight = false;
	bIsFiring = false;
}

void ASWATEnemyCharacter::EnableHitReaction()
{
	bIsHitReactionOnCooldown = false;
}

void ASWATEnemyCharacter::LockMovementForHitReaction()
{
	bIsHitReacting = true;
	BroadcastStateChanged();

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	if (MovementComponent)
	{
		PreviousMovementMode = MovementComponent->MovementMode;
		PreviousCustomMovementMode = MovementComponent->CustomMovementMode;
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
	}

	AController* CurrentController = GetController();

	if (CurrentController)
	{
		CurrentController->StopMovement();
	}
}

void ASWATEnemyCharacter::RestoreMovementAfterHitReaction()
{
	if (bIsDead)
	{
		return;
	}

	bIsHitReacting = false;
	BroadcastStateChanged();

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	if (MovementComponent)
	{
		MovementComponent->SetMovementMode(
			PreviousMovementMode,
			PreviousCustomMovementMode
		);
	}
}

void ASWATEnemyCharacter::BroadcastStateChanged()
{
	OnSWATStateChanged.Broadcast();
}
