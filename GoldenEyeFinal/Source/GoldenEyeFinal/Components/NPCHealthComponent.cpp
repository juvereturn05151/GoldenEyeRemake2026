#include "NPCHealthComponent.h"

UNPCHealthComponent::UNPCHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNPCHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	bIsDead = false;

	BroadcastHealthChanged();
}

void UNPCHealthComponent::ApplyDamage(float DamageAmount)
{
	if (bIsDead || DamageAmount <= 0.0f)
	{
		return;
	}

	const float PreviousHealth = CurrentHealth;

	CurrentHealth = FMath::Clamp(
		CurrentHealth - DamageAmount,
		0.0f,
		MaxHealth
	);

	const float AppliedDamage = PreviousHealth - CurrentHealth;

	if (AppliedDamage <= 0.0f)
	{
		return;
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[NPC Health] Owner=%s AppliedDamage=%.2f PreviousHealth=%.2f CurrentHealth=%.2f MaxHealth=%.2f"),
		*GetNameSafe(GetOwner()),
		AppliedDamage,
		PreviousHealth,
		CurrentHealth,
		MaxHealth
	);

	OnDamageTaken.Broadcast(AppliedDamage);
	BroadcastHealthChanged();

	if (CurrentHealth <= 0.0f)
	{
		bIsDead = true;
		OnDeath.Broadcast();
	}
}

float UNPCHealthComponent::GetCurrentHealth() const
{
	return CurrentHealth;
}

float UNPCHealthComponent::GetMaxHealth() const
{
	return MaxHealth;
}

float UNPCHealthComponent::GetHealthPercent() const
{
	return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

bool UNPCHealthComponent::IsDead() const
{
	return bIsDead;
}

void UNPCHealthComponent::BroadcastHealthChanged()
{
	OnHealthChanged.Broadcast(
		CurrentHealth,
		MaxHealth,
		GetHealthPercent()
	);
}
