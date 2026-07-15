#include "BondHealthComponent.h"

#include "Engine/World.h"
#include "TimerManager.h"

namespace
{
	constexpr float RegenerationTickInterval = 0.05f;
}

UBondHealthComponent::UBondHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBondHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	bIsDead = false;
	bIsRegenerating = false;

	BroadcastHealthChanged();
}

void UBondHealthComponent::ApplyDamage(float DamageAmount)
{
	if (bIsDead || DamageAmount <= 0.0f)
	{
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	StopRegeneration();
	World->GetTimerManager().ClearTimer(RegenerationDelayTimer);

	const float PreviousHealth = CurrentHealth;

	CurrentHealth = FMath::Clamp(
		CurrentHealth - DamageAmount,
		0.0f,
		MaxHealth
	);

	const float AppliedDamage = PreviousHealth - CurrentHealth;

	OnDamageTaken.Broadcast(AppliedDamage);
	BroadcastHealthChanged();

	if (CurrentHealth <= 0.0f)
	{
		bIsDead = true;
		OnDeath.Broadcast();
		return;
	}

	ScheduleRegeneration();
}

void UBondHealthComponent::RestoreFullHealth()
{
	if (bIsDead)
	{
		return;
	}

	UWorld* World = GetWorld();

	if (World)
	{
		World->GetTimerManager().ClearTimer(RegenerationDelayTimer);
	}

	StopRegeneration();

	CurrentHealth = MaxHealth;
	BroadcastHealthChanged();
}

float UBondHealthComponent::GetCurrentHealth() const
{
	return CurrentHealth;
}

float UBondHealthComponent::GetMaxHealth() const
{
	return MaxHealth;
}

float UBondHealthComponent::GetHealthPercent() const
{
	return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

float UBondHealthComponent::GetDamagePercent() const
{
	return 1.0f - GetHealthPercent();
}

bool UBondHealthComponent::IsDead() const
{
	return bIsDead;
}

bool UBondHealthComponent::IsRegenerating() const
{
	return bIsRegenerating;
}

bool UBondHealthComponent::IsLowHealth() const
{
	return !bIsDead && GetHealthPercent() <= LowHealthThreshold;
}

void UBondHealthComponent::ScheduleRegeneration()
{
	if (bIsDead || CurrentHealth >= MaxHealth || RegenerationRate <= 0.0f)
	{
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		RegenerationDelayTimer,
		this,
		&UBondHealthComponent::StartRegeneration,
		RegenerationDelay,
		false
	);
}

void UBondHealthComponent::StartRegeneration()
{
	if (
		bIsDead ||
		bIsRegenerating ||
		CurrentHealth >= MaxHealth ||
		RegenerationRate <= 0.0f
		)
	{
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	bIsRegenerating = true;
	OnRegenerationStateChanged.Broadcast(true);

	World->GetTimerManager().SetTimer(
		RegenerationTickTimer,
		this,
		&UBondHealthComponent::RegenerateHealth,
		RegenerationTickInterval,
		true
	);
}

void UBondHealthComponent::RegenerateHealth()
{
	if (bIsDead || CurrentHealth >= MaxHealth)
	{
		StopRegeneration();
		return;
	}

	CurrentHealth = FMath::Clamp(
		CurrentHealth + (RegenerationRate * RegenerationTickInterval),
		0.0f,
		MaxHealth
	);

	BroadcastHealthChanged();

	if (CurrentHealth >= MaxHealth)
	{
		StopRegeneration();
	}
}

void UBondHealthComponent::StopRegeneration()
{
	UWorld* World = GetWorld();

	if (World)
	{
		World->GetTimerManager().ClearTimer(RegenerationTickTimer);
	}

	if (bIsRegenerating)
	{
		bIsRegenerating = false;
		OnRegenerationStateChanged.Broadcast(false);
	}
}

void UBondHealthComponent::BroadcastHealthChanged()
{
	OnHealthChanged.Broadcast(
		CurrentHealth,
		MaxHealth,
		GetHealthPercent()
	);
}
