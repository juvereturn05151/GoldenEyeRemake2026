/*
Name: Ju-ve Chankasemporn
E-mail: juvereturn@gmail.com
@2026 MyLoyalFans Productions
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FNPCHealthChangedSignature,
	float,
	CurrentHealth,
	float,
	MaxHealth,
	float,
	HealthPercent
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FNPCDamageTakenSignature,
	float,
	DamageAmount
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNPCDeathSignature);

UCLASS(ClassGroup = (NPC), meta = (BlueprintSpawnableComponent))
class GOLDENEYEFINAL_API UNPCHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNPCHealthComponent();

	UFUNCTION(BlueprintCallable, Category = "NPC|Health")
	void ApplyDamage(float DamageAmount);

	UFUNCTION(BlueprintPure, Category = "NPC|Health")
	float GetCurrentHealth() const;

	UFUNCTION(BlueprintPure, Category = "NPC|Health")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintPure, Category = "NPC|Health")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "NPC|Health")
	bool IsDead() const;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Health")
	FNPCHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Health")
	FNPCDamageTakenSignature OnDamageTaken;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Health")
	FNPCDeathSignature OnDeath;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Health", meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "NPC|Health", meta = (AllowPrivateAccess = "true"))
	float CurrentHealth = 100.0f;

	bool bIsDead = false;

	void BroadcastHealthChanged();
};
