#include "SWATEnemyCharacter.h"

#include "../Components/NPCHealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "TimerManager.h"

ASWATEnemyCharacter::ASWATEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	HealthComponent = CreateDefaultSubobject<UNPCHealthComponent>(TEXT("HealthComponent"));
}

void ASWATEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

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

bool ASWATEnemyCharacter::IsDead() const
{
	return bIsDead;
}

void ASWATEnemyCharacter::SetInCombat(bool bNewIsInCombat)
{
	bIsInCombat = !bIsDead && bNewIsInCombat;
}

void ASWATEnemyCharacter::SetReloading(bool bNewIsReloading)
{
	bIsReloading = !bIsDead && bNewIsReloading;
}

void ASWATEnemyCharacter::SetHasLineOfSight(bool bNewHasLineOfSight)
{
	bHasLineOfSight = !bIsDead && bNewHasLineOfSight;
}

void ASWATEnemyCharacter::SetFiring(bool bNewIsFiring)
{
	bIsFiring = !bIsDead && bNewIsFiring;
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

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	if (MovementComponent)
	{
		MovementComponent->SetMovementMode(
			PreviousMovementMode,
			PreviousCustomMovementMode
		);
	}
}
