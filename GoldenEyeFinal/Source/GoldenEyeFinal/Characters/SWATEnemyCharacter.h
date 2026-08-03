/*
Name: Ju-ve Chankasemporn
E-mail: juvereturn@gmail.com
@2026 MyLoyalFans Productions
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SWATEnemyCharacter.generated.h"

class UNPCHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSWATStateChangedSignature);

UCLASS()
class GOLDENEYEFINAL_API ASWATEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASWATEnemyCharacter();

	UFUNCTION(BlueprintPure, Category = "SWAT|Components")
	UNPCHealthComponent* GetHealthComponent() const;

	UFUNCTION(BlueprintPure, Category = "SWAT|State")
	bool IsDead() const;

	UFUNCTION(BlueprintPure, Category = "SWAT|State")
	bool IsHitReacting() const;

	UFUNCTION(BlueprintPure, Category = "SWAT|State")
	bool IsInCombat() const;

	UPROPERTY(BlueprintAssignable, Category = "SWAT|State")
	FSWATStateChangedSignature OnSWATStateChanged;

	UFUNCTION(BlueprintCallable, Category = "SWAT|State")
	void SetInCombat(bool bNewIsInCombat);

	UFUNCTION(BlueprintCallable, Category = "SWAT|State")
	void SetReloading(bool bNewIsReloading);

	UFUNCTION(BlueprintCallable, Category = "SWAT|State")
	void SetHasLineOfSight(bool bNewHasLineOfSight);

	UFUNCTION(BlueprintCallable, Category = "SWAT|State")
	void SetFiring(bool bNewIsFiring);

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "SWAT|State")
	void OnSWATDeath();

	UFUNCTION(BlueprintImplementableEvent, Category = "SWAT|State")
	void OnSWATHitReaction(float DamageAmount);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SWAT|Components")
	TObjectPtr<UNPCHealthComponent> HealthComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|State", meta = (ClampMin = "0.0"))
	float HitReactionCooldown = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|State", meta = (ClampMin = "0.0"))
	float HitReactionMovementLockDuration = 0.30f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SWAT|State")
	bool bIsDead = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SWAT|State")
	bool bIsHitReacting = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SWAT|State")
	bool bIsInCombat = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SWAT|State")
	bool bIsReloading = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SWAT|State")
	bool bHasLineOfSight = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SWAT|State")
	bool bIsFiring = false;

private:
	UFUNCTION()
	void HandleDeath();

	UFUNCTION()
	void HandleDamageTaken(float DamageAmount);

	void StopMovementOnDeath();
	void StopCombatOnDeath();
	void EnableHitReaction();
	void LockMovementForHitReaction();
	void RestoreMovementAfterHitReaction();
	void BroadcastStateChanged();

	bool bIsHitReactionOnCooldown = false;
	TEnumAsByte<EMovementMode> PreviousMovementMode = MOVE_Walking;
	uint8 PreviousCustomMovementMode = 0;
	FTimerHandle HitReactionCooldownTimer;
	FTimerHandle HitReactionMovementLockTimer;
};
