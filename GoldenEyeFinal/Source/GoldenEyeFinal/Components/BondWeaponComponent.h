/*
Name: Ju-ve Chankasemporn
E-mail: juvereturn@gmail.com
@2026 MyLoyalFans Productions
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BondWeaponComponent.generated.h"

class ABondWeaponBase;

UCLASS(ClassGroup = (Bond), meta = (BlueprintSpawnableComponent))
class GOLDENEYEFINAL_API UBondWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBondWeaponComponent();

	UFUNCTION(BlueprintCallable, Category = "Bond|Weapon")
	void SpawnDefaultWeapon();

	UFUNCTION(BlueprintCallable, Category = "Bond|Weapon")
	void EquipWeapon(ABondWeaponBase* NewWeapon);

	UFUNCTION(BlueprintCallable, Category = "Bond|Weapon")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Bond|Weapon")
	void StopFire();

	UFUNCTION(BlueprintCallable, Category = "Bond|Weapon")
	void Reload();

	UFUNCTION(BlueprintPure, Category = "Bond|Weapon")
	ABondWeaponBase* GetEquippedWeapon() const;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Weapon", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ABondWeaponBase> DefaultWeaponClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bond|Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ABondWeaponBase> EquippedWeapon;
};
