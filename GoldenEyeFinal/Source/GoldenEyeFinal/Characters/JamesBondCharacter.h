/*
Name: Ju-ve Chankasemporn
E-mail: juvereturn@gmail.com
@2026 MyLoyalFans Productions
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "JamesBondCharacter.generated.h"

class UAudioComponent;
class UBondFootstepComponent;
class UBondHealthComponent;
class UBondTimeSlowComponent;
class UBondWeaponComponent;
class UCameraComponent;
class UInputAction;
class UInputComponent;
class UInputMappingContext;
class USceneComponent;
class USkeletalMeshComponent;
class ACopyOpportunity;
class APhotoOpportunity;

UCLASS()
class GOLDENEYEFINAL_API AJamesBondCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AJamesBondCharacter();

	UFUNCTION(BlueprintPure, Category = "Bond|Components")
	USceneComponent* GetWeaponRoot() const;

	UFUNCTION(BlueprintPure, Category = "Bond|Components")
	USkeletalMeshComponent* GetFirstPersonArms() const;

	UFUNCTION(BlueprintPure, Category = "Bond|Components")
	UBondHealthComponent* GetHealthComponent() const;

	UFUNCTION(BlueprintPure, Category = "Bond|Components")
	UBondWeaponComponent* GetWeaponComponent() const;

	UFUNCTION(BlueprintPure, Category = "Bond|Components")
	UBondTimeSlowComponent* GetTimeSlowComponent() const;

	UFUNCTION(BlueprintPure, Category = "Bond|Components")
	UBondFootstepComponent* GetFootstepComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Bond|Weapon")
	void CompleteReload();

	UFUNCTION(BlueprintCallable, Category = "Bond|Interactions")
	void SetCopyOpportunity(ACopyOpportunity* CopyOpportunity);

	UFUNCTION(BlueprintCallable, Category = "Bond|Interactions")
	void ClearCopyOpportunity(ACopyOpportunity* CopyOpportunity);

	UFUNCTION(BlueprintCallable, Category = "Bond|Interactions")
	void TryCopyInteraction();

	UFUNCTION(BlueprintCallable, Category = "Bond|Interactions")
	void SetPhotoOpportunity(APhotoOpportunity* PhotoOpportunity);

	UFUNCTION(BlueprintCallable, Category = "Bond|Interactions")
	void ClearPhotoOpportunity(APhotoOpportunity* PhotoOpportunity);

	UFUNCTION(BlueprintCallable, Category = "Bond|Interactions")
	void TakePhoto();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bond|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bond|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> FirstPersonArms;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bond|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> WeaponRoot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Weapon", meta = (AllowPrivateAccess = "true"))
	FName WeaponRootSocketName = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bond|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAudioComponent> BondAudioComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bond|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBondHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bond|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBondWeaponComponent> WeaponComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bond|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBondTimeSlowComponent> TimeSlowComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bond|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBondFootstepComponent> FootstepComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ReloadAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> TimeSlowAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> TakePhotoAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ToggleMissionPanelAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> InteractionAction;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bond|Interactions", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ACopyOpportunity> CurrentCopyOpportunity;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bond|Interactions", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<APhotoOpportunity> CurrentPhotoOpportunity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Death", meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
	float DeathFallDuration = 1.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Death", meta = (AllowPrivateAccess = "true"))
	FVector DeathCameraLocationOffset = FVector(25.0f, 12.0f, -58.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Death", meta = (AllowPrivateAccess = "true"))
	FRotator DeathCameraRotationOffset = FRotator(-68.0f, 0.0f, 18.0f);

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartJump();
	void StopJump();
	void HandleFireStarted();
	void HandleFireCompleted();
	void HandleReload();
	void HandleTimeSlowStarted();
	void HandleTimeSlowCompleted();
	void HandleInteraction();
	void HandleToggleMissionPanel();
	void AttachWeaponRootToConfiguredSocket();
	void InitializeInputMapping();

	UFUNCTION()
	void HandleDeath();

	void StartDeathFall();
	void UpdateDeathFall(float DeltaTime);
	void DisableBondOnDeath();

	bool bIsDeathFalling = false;
	float DeathFallElapsed = 0.0f;
	FVector DeathCameraStartLocation = FVector::ZeroVector;
	FVector DeathCameraTargetLocation = FVector::ZeroVector;
	FRotator DeathCameraStartRotation = FRotator::ZeroRotator;
	FRotator DeathCameraTargetRotation = FRotator::ZeroRotator;
};
