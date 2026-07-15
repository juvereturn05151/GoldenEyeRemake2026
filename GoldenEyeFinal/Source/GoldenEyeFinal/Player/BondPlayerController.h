/*
Name: Ju-ve Chankasemporn
E-mail: juvereturn@gmail.com
@2026 MyLoyalFans Productions
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BondPlayerController.generated.h"

class AJamesBondCharacter;
class UUserWidget;

UCLASS()
class GOLDENEYEFINAL_API ABondPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABondPlayerController();

	UFUNCTION(BlueprintPure, Category = "Bond|UI")
	UUserWidget* GetMainHUDWidget() const;

	UFUNCTION(BlueprintPure, Category = "Bond|UI")
	UUserWidget* GetTimeSlowHUDWidget() const;

	UFUNCTION(BlueprintCallable, Category = "Bond|UI")
	void BindPossessedBondDelegates();

	UFUNCTION(BlueprintImplementableEvent, Category = "Bond|UI")
	void OnPlayerWidgetsCreated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Bond|UI")
	void UpdateDamageState(float CurrentHealth,float MaxHealth,float HealthPercent);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bond|UI")
	void PlayDamageFlash(float DamageAmount);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bond|UI")
	void SetRegenerating(bool bIsRegenerating);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bond|UI")
	void HandleBondDeath();

	UFUNCTION(BlueprintImplementableEvent, Category = "Bond|UI")
	void UpdateAmmo(int32 MagazineAmmo, int32 ReserveAmmo);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bond|UI")
	void UpdateTimeSlowMeter(float CurrentMeter, float MaxMeter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bond|UI")
	void SetTimeSlowActive(bool bIsActive);

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

private:
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Bond|UI",
		meta = (AllowPrivateAccess = "true")
	)
	TSubclassOf<UUserWidget> MainHUDWidgetClass;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Bond|UI",
		meta = (AllowPrivateAccess = "true")
	)
	TSubclassOf<UUserWidget> TimeSlowHUDWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> MainHUDWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> TimeSlowHUDWidget;

	UPROPERTY()
	TObjectPtr<AJamesBondCharacter> PossessedBond;

	FTimerHandle DeferredWeaponBindingTimer;

	void CreatePlayerWidgets();
	void BindWeaponDelegates();

	UFUNCTION()
	void HandleHealthChanged(
		float CurrentHealth,
		float MaxHealth,
		float HealthPercent
	);

	UFUNCTION()
	void HandleDamageTaken(float DamageAmount);

	UFUNCTION()
	void HandleRegenerationStateChanged(bool bIsRegenerating);

	UFUNCTION()
	void HandleDeath();

	UFUNCTION()
	void HandleAmmoChanged(int32 MagazineAmmo, int32 ReserveAmmo);

	UFUNCTION()
	void HandleTimeSlowMeterChanged(float CurrentMeter, float MaxMeter);

	UFUNCTION()
	void HandleTimeSlowStateChanged(bool bIsActive);
};
