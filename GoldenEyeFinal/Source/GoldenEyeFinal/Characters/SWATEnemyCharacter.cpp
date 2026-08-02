#include "SWATEnemyCharacter.h"

#include "../Components/NPCHealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"

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

	StopMovementOnDeath();
	StopCombatOnDeath();
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
