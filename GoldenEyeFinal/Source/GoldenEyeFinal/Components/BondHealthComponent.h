/*
Name: Ju-ve Chankasemporn
E-mail: juvereturn@gmail.com
@2026 MyLoyalFans Productions
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BondHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FBondHealthChangedSignature,
	float,
	CurrentHealth,
	float,
	MaxHealth,
	float,
	HealthPercent
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FBondDamageTakenSignature,
	float,
	DamageAmount
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FBondRegenerationStateChangedSignature,
	bool,
	bIsRegenerating
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FBondDeathSignature
);

UCLASS(ClassGroup = (Bond), meta = (BlueprintSpawnableComponent))
class GOLDENEYEFINAL_API UBondHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBondHealthComponent();

	UFUNCTION(BlueprintCallable, Category = "Bond|Health")
	void ApplyDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Bond|Health")
	void RestoreFullHealth();

	UFUNCTION(BlueprintPure, Category = "Bond|Health")
	float GetCurrentHealth() const;

	UFUNCTION(BlueprintPure, Category = "Bond|Health")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintPure, Category = "Bond|Health")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Bond|Health")
	float GetDamagePercent() const;

	UFUNCTION(BlueprintPure, Category = "Bond|Health")
	bool IsDead() const;

	UFUNCTION(BlueprintPure, Category = "Bond|Health")
	bool IsRegenerating() const;

	UFUNCTION(BlueprintPure, Category = "Bond|Health")
	bool IsLowHealth() const;

	UPROPERTY(BlueprintAssignable, Category = "Bond|Health")
	FBondHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Bond|Health")
	FBondDamageTakenSignature OnDamageTaken;

	UPROPERTY(BlueprintAssignable, Category = "Bond|Health")
	FBondRegenerationStateChangedSignature OnRegenerationStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Bond|Health")
	FBondDeathSignature OnDeath;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Bond|Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleInstanceOnly, Category = "Bond|Health")
	float CurrentHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Bond|Health|Regeneration", meta = (ClampMin = "0.0"))
	float RegenerationDelay = 4.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Bond|Health|Regeneration", meta = (ClampMin = "0.0"))
	float RegenerationRate = 25.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Bond|Health", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LowHealthThreshold = 0.30f;

	bool bIsDead = false;
	bool bIsRegenerating = false;

	FTimerHandle RegenerationDelayTimer;
	FTimerHandle RegenerationTickTimer;

	void ScheduleRegeneration();
	void StartRegeneration();
	void RegenerateHealth();
	void StopRegeneration();
	void BroadcastHealthChanged();
};
