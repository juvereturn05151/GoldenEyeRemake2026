#include "SWATEnemyCharacter.h"

#include "../Components/NPCHealthComponent.h"
#include "../Components/SWATCombatComponent.h"
#include "../Components/SWATWeaponComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/DamageEvents.h"
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
		MovementComponent->bUseRVOAvoidance = true;
		MovementComponent->AvoidanceConsiderationRadius = 400.0f;
		MovementComponent->AvoidanceWeight = 0.5f;
		MovementComponent->RepulsionForce = 0.0f;
		MovementComponent->bEnablePhysicsInteraction = false;
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
		MovementComponent->bUseRVOAvoidance = true;
		MovementComponent->AvoidanceConsiderationRadius = 400.0f;
		MovementComponent->AvoidanceWeight = 0.5f;
		MovementComponent->RepulsionForce = 0.0f;
		MovementComponent->bEnablePhysicsInteraction = false;
	}

	if (HealthComponent)
	{
		HealthComponent->OnDamageTaken.AddUniqueDynamic(
			this,
			&ASWATEnemyCharacter::HandleDamageTaken
		);

		HealthComponent->OnDeath.AddUniqueDynamic(
			this,
			&ASWATEnemyCharacter::HandleDeath
		);
	}
}

void ASWATEnemyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DeathCleanupTimer);
	}

	Super::EndPlay(EndPlayReason);
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

bool ASWATEnemyCharacter::IsFiring() const
{
	return bIsFiring;
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

	if (bIsFiring)
	{
		StopMovementForLockedState();
	}

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

float ASWATEnemyCharacter::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser
)
{
	if (bIsDead || !HealthComponent || DamageAmount <= 0.0f)
	{
		return 0.0f;
	}

	FName BoneName = NAME_None;
	UPrimitiveComponent* HitComponent = nullptr;

	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent* PointDamageEvent =
			static_cast<const FPointDamageEvent*>(&DamageEvent);

		BoneName = PointDamageEvent->HitInfo.BoneName;
		HitComponent = PointDamageEvent->HitInfo.GetComponent();
	}

	ApplySWATDamage(DamageAmount, BoneName, HitComponent);

	return DamageAmount;
}

void ASWATEnemyCharacter::ApplySWATDamage(
	float Damage,
	FName BoneName,
	UPrimitiveComponent* HitComponent
)
{
	if (bIsDead || !HealthComponent || Damage <= 0.0f)
	{
		return;
	}

	float FinalDamage = Damage;
	const bool bIsHeadshot = IsHeadshotBone(BoneName);

	if (bIsHeadshot)
	{
		FinalDamage = bInstantKillHeadshots
			? HealthComponent->GetCurrentHealth()
			: Damage * HeadshotDamageMultiplier;
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[SWAT Damage] Headshot=%s Bone=%s BaseDamage=%.2f FinalDamage=%.2f HitComponent=%s"),
		bIsHeadshot ? TEXT("TRUE") : TEXT("FALSE"),
		*BoneName.ToString(),
		Damage,
		FinalDamage,
		*GetNameSafe(HitComponent)
	);

	HealthComponent->ApplyDamage(FinalDamage);
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
	ScheduleDeathCleanup();
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

bool ASWATEnemyCharacter::IsHeadshotBone(FName BoneName) const
{
	if (BoneName.IsNone())
	{
		return false;
	}

	for (const FName HeadshotBoneName : HeadshotBoneNames)
	{
		if (BoneName.IsEqual(HeadshotBoneName, ENameCase::IgnoreCase))
		{
			return true;
		}
	}

	return false;
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

void ASWATEnemyCharacter::StopMovementForLockedState()
{
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	if (AController* CurrentController = GetController())
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

void ASWATEnemyCharacter::ScheduleDeathCleanup()
{
	UWorld* World = GetWorld();

	if (!World)
	{
		DisableCollisionAndDestroy();
		return;
	}

	World->GetTimerManager().ClearTimer(DeathCleanupTimer);

	if (DeathDestroyDelay <= 0.0f)
	{
		DisableCollisionAndDestroy();
		return;
	}

	World->GetTimerManager().SetTimer(
		DeathCleanupTimer,
		this,
		&ASWATEnemyCharacter::DisableCollisionAndDestroy,
		DeathDestroyDelay,
		false
	);
}

void ASWATEnemyCharacter::DisableCollisionAndDestroy()
{
	DisableAllCollision();
	Destroy();
}

void ASWATEnemyCharacter::DisableAllCollision()
{
	SetActorEnableCollision(false);

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	GetComponents<UPrimitiveComponent>(PrimitiveComponents);

	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!PrimitiveComponent)
		{
			continue;
		}

		PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PrimitiveComponent->SetGenerateOverlapEvents(false);
	}
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
