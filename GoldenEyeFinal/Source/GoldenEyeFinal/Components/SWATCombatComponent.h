#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SWATCombatComponent.generated.h"

class ASWATEnemyCharacter;
class USWATWeaponComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSWATBurstSignature);

UCLASS(ClassGroup = (SWAT), meta = (BlueprintSpawnableComponent))
class GOLDENEYEFINAL_API USWATCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USWATCombatComponent();

	UFUNCTION(BlueprintCallable, Category = "SWAT|Combat")
	bool StartCombatBurst(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "SWAT|Combat")
	void StopCombatBurst();

	UFUNCTION(BlueprintPure, Category = "SWAT|Combat")
	bool CanStartBurst() const;

	UFUNCTION(BlueprintPure, Category = "SWAT|Combat")
	bool IsBurstActive() const;

	UFUNCTION(BlueprintPure, Category = "SWAT|Combat")
	bool IsBurstOnCooldown() const;

	UFUNCTION(BlueprintCallable, Category = "SWAT|Combat")
	void SetHasLineOfSight(bool bNewHasLineOfSight);

	UPROPERTY(BlueprintAssignable, Category = "SWAT|Combat")
	FSWATBurstSignature OnBurstStarted;

	UPROPERTY(BlueprintAssignable, Category = "SWAT|Combat")
	FSWATBurstSignature OnBurstShot;

	UPROPERTY(BlueprintAssignable, Category = "SWAT|Combat")
	FSWATBurstSignature OnBurstFinished;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|Combat", meta = (ClampMin = "0.0"))
	float AimPreparationTime = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|Combat", meta = (ClampMin = "1"))
	int32 MinimumBurstShots = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|Combat", meta = (ClampMin = "1"))
	int32 MaximumBurstShots = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|Combat", meta = (ClampMin = "0.0"))
	float TimeBetweenShots = 0.12f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|Combat", meta = (ClampMin = "0.0"))
	float BurstCooldown = 0.8f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SWAT|Combat")
	TObjectPtr<AActor> CurrentTarget;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SWAT|Combat")
	int32 RemainingBurstShots = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SWAT|Combat")
	bool bBurstActive = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SWAT|Combat")
	bool bBurstOnCooldown = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SWAT|Combat")
	bool bHasLineOfSight = false;

private:
	UFUNCTION()
	void HandleOwnerStateChanged();

	UFUNCTION()
	void HandleWeaponAmmoChanged(int32 MagazineAmmo, int32 ReserveAmmo);

	UFUNCTION()
	void HandleWeaponReloadStarted();

	void BeginBurstShots();
	void FireNextBurstShot();
	void FinishBurst(bool bStartCooldown);
	void FinishBurstCooldown();
	bool ValidateBurstContinuation() const;
	ASWATEnemyCharacter* GetSWATOwner() const;
	USWATWeaponComponent* GetWeaponComponent() const;
	void ClearBurstTimers();
	void BindOwnerAndWeapon();
	void UnbindOwnerAndWeapon();

	FTimerHandle AimDelayTimerHandle;
	FTimerHandle ShotIntervalTimerHandle;
	FTimerHandle BurstCooldownTimerHandle;
};
