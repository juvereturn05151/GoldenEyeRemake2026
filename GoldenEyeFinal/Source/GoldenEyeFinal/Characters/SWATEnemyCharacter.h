/*
Name: Ju-ve Chankasemporn
E-mail: juvereturn@gmail.com
@2026 MyLoyalFans Productions
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SWATEnemyCharacter.generated.h"

class UNPCHealthComponent;

UCLASS()
class GOLDENEYEFINAL_API ASWATEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASWATEnemyCharacter();

	UFUNCTION(BlueprintPure, Category = "SWAT|Components")
	UNPCHealthComponent* GetHealthComponent() const;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SWAT|Components")
	TObjectPtr<UNPCHealthComponent> HealthComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SWAT|State")
	bool bIsDead = false;

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

	void StopMovementOnDeath();
	void StopCombatOnDeath();
};
