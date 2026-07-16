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
class UBondHealthComponent;
class UBondTimeSlowComponent;
class UBondWeaponComponent;
class UCameraComponent;
class UInputAction;
class UInputComponent;
class UInputMappingContext;
class USceneComponent;
class USkeletalMeshComponent;

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

	UFUNCTION(BlueprintCallable, Category = "Bond|Weapon")
	void CompleteReload();

protected:
	virtual void BeginPlay() override;

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

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartJump();
	void StopJump();
	void HandleFireStarted();
	void HandleFireCompleted();
	void HandleReload();
	void HandleTimeSlowStarted();
	void HandleTimeSlowCompleted();
	void AttachWeaponRootToConfiguredSocket();
	void InitializeInputMapping();
};
