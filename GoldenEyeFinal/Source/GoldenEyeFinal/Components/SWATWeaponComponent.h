#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SWATWeaponComponent.generated.h"

class ASWATProjectile;
class USceneComponent;
class USkeletalMeshComponent;
class USoundBase;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FSWATAmmoChangedSignature,
	int32,
	MagazineAmmo,
	int32,
	ReserveAmmo
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FSWATProjectileFiredSignature,
	ASWATProjectile*,
	Projectile
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSWATReloadSignature);

UCLASS(ClassGroup = (SWAT), meta = (BlueprintSpawnableComponent))
class GOLDENEYEFINAL_API USWATWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USWATWeaponComponent();

	UFUNCTION(BlueprintCallable, Category = "SWAT|Weapon")
	void SetWeaponMesh(USkeletalMeshComponent* InWeaponMesh);

	UFUNCTION(BlueprintCallable, Category = "SWAT|Weapon")
	void SetWeaponSceneComponent(USceneComponent* InWeaponMesh);

	UFUNCTION(BlueprintCallable, Category = "SWAT|Weapon")
	void SetStaticWeaponMesh(UStaticMeshComponent* InWeaponMesh);

	UFUNCTION(BlueprintCallable, Category = "SWAT|Weapon")
	bool FireProjectileAt(AActor* Target);

	void EnableDebugDrawForNextShot();

	UFUNCTION(BlueprintPure, Category = "SWAT|Weapon")
	bool CanFire() const;

	UFUNCTION(BlueprintCallable, Category = "SWAT|Weapon")
	bool StartReload();

	UFUNCTION(BlueprintCallable, Category = "SWAT|Weapon")
	void CompleteReload();

	UFUNCTION(BlueprintCallable, Category = "SWAT|Weapon")
	void CancelReload();

	UFUNCTION(BlueprintCallable, Category = "SWAT|Weapon")
	void StopWeapon();

	UFUNCTION(BlueprintPure, Category = "SWAT|Weapon")
	bool NeedsReload() const;

	UFUNCTION(BlueprintPure, Category = "SWAT|Weapon")
	bool IsReloading() const;

	UFUNCTION(BlueprintPure, Category = "SWAT|Weapon")
	int32 GetMagazineAmmo() const;

	UFUNCTION(BlueprintPure, Category = "SWAT|Weapon")
	int32 GetReserveAmmo() const;

	UPROPERTY(BlueprintAssignable, Category = "SWAT|Weapon")
	FSWATAmmoChangedSignature OnAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "SWAT|Weapon")
	FSWATProjectileFiredSignature OnProjectileFired;

	UPROPERTY(BlueprintAssignable, Category = "SWAT|Weapon")
	FSWATReloadSignature OnReloadStarted;

	UPROPERTY(BlueprintAssignable, Category = "SWAT|Weapon")
	FSWATReloadSignature OnReloadFinished;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|Weapon")
	TSubclassOf<ASWATProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|Weapon", meta = (ClampMin = "1"))
	int32 MagazineCapacity = 12;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|Weapon", meta = (ClampMin = "0"))
	int32 StartingReserveAmmo = 36;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|Weapon", meta = (ClampMin = "0.0"))
	float ReloadDuration = 1.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|Weapon")
	FName MuzzleSocketName = TEXT("Muzzle");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|Weapon|Audio")
	TObjectPtr<USoundBase> FireSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|Weapon", meta = (ClampMin = "0.0"))
	float BaseSpreadDegrees = 2.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|Weapon")
	float TargetAimHeightOffset = 50.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SWAT|Weapon")
	int32 CurrentMagazineAmmo = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SWAT|Weapon")
	int32 CurrentReserveAmmo = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SWAT|Weapon")
	bool bIsReloading = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SWAT|Weapon")
	TObjectPtr<USceneComponent> WeaponMesh;

private:
	UFUNCTION()
	void HandleOwnerStateChanged();

	bool CanOwnerUseWeapon() const;
	void BroadcastAmmoChanged();
	void SetReloadingState(bool bNewIsReloading);

	FTimerHandle ReloadTimerHandle;
	bool bDrawDebugForNextShot = false;
};
