/*
Name: Ju-ve Chankasemporn
E-mail: juvereturn@gmail.com
@2026 MyLoyalFans Productions
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BondWeaponBase.generated.h"

class USceneComponent;
class USkeletalMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FBondAmmoChangedSignature,
	int32,
	MagazineAmmo,
	int32,
	ReserveAmmo
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBondReloadSignature);

UCLASS()
class GOLDENEYEFINAL_API ABondWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	ABondWeaponBase();

	UFUNCTION(BlueprintCallable, Category = "Bond|Weapon")
	virtual void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Bond|Weapon")
	virtual void StopFire();

	UFUNCTION(BlueprintCallable, Category = "Bond|Weapon")
	virtual void StartReload();

	UFUNCTION(BlueprintCallable, Category = "Bond|Weapon")
	virtual void CompleteReload();

	UFUNCTION(BlueprintPure, Category = "Bond|Weapon")
	bool CanFire() const;

	UFUNCTION(BlueprintPure, Category = "Bond|Weapon")
	bool CanReload() const;

	UFUNCTION(BlueprintPure, Category = "Bond|Weapon")
	USceneComponent* GetMuzzlePoint() const;

	UFUNCTION(BlueprintPure, Category = "Bond|Weapon")
	FName GetMuzzleSocketName() const;

	UPROPERTY(BlueprintAssignable, Category = "Bond|Weapon")
	FBondAmmoChangedSignature OnAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "Bond|Weapon")
	FBondReloadSignature OnReloadStarted;

	UPROPERTY(BlueprintAssignable, Category = "Bond|Weapon")
	FBondReloadSignature OnReloadFinished;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bond|Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bond|Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bond|Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> MuzzlePoint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Weapon", meta = (AllowPrivateAccess = "true"))
	FName MuzzleSocketName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Weapon|Ammo", meta = (AllowPrivateAccess = "true", ClampMin = "1"))
	int32 MagazineCapacity = 8;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bond|Weapon|Ammo", meta = (AllowPrivateAccess = "true"))
	int32 MagazineAmmo = 8;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Weapon|Ammo", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	int32 StartingReserveAmmo = 32;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bond|Weapon|Ammo", meta = (AllowPrivateAccess = "true"))
	int32 ReserveAmmo = 32;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Weapon", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float FireInterval = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Weapon|Reload", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float ReloadDuration = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Weapon", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float Damage = 25.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Weapon", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float TraceRange = 10000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Weapon", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Weapon|Debug", meta = (AllowPrivateAccess = "true"))
	bool bDrawDebugTrace = false;

	bool bIsFiring = false;
	bool bIsReloading = false;
	float LastFireTime = -BIG_NUMBER;

	FTimerHandle FireTimer;
	FTimerHandle ReloadTimer;

	void FireOnce();
	FVector GetTraceStart() const;
	FVector GetTraceDirection() const;
	void BroadcastAmmoChanged();
};
