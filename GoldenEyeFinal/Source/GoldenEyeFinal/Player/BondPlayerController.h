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
class UMissionObjective;
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

	UFUNCTION(BlueprintPure, Category = "Bond|UI")
	UUserWidget* GetDeathWidget() const;

	UFUNCTION(BlueprintPure, Category = "Bond|UI")
	UUserWidget* GetWinWidget() const;

	UFUNCTION(BlueprintPure, Category = "Bond|UI")
	UUserWidget* GetInteractionPromptWidget() const;

	UFUNCTION(BlueprintCallable, Category = "Bond|UI")
	void BindPossessedBondDelegates();

	UFUNCTION(BlueprintCallable, Category = "Bond|UI")
	void ShowInteractionPrompt(const FText& PromptText);

	UFUNCTION(BlueprintCallable, Category = "Bond|UI")
	void HideInteractionPrompt();

	UFUNCTION(BlueprintCallable, Category = "Bond|UI")
	void ShowWinWidget(bool bPauseGame = true);

	UFUNCTION(BlueprintCallable, Category = "Bond|Mission UI")
	void InitializeMissionObjectiveUI();

	UFUNCTION(BlueprintCallable, Category = "Bond|Mission UI")
	void ToggleMissionPanel();

	UFUNCTION(BlueprintCallable, Category = "Bond|Mission UI")
	void SetMissionPanelVisible(bool bVisible);

	UFUNCTION(BlueprintPure, Category = "Bond|Mission UI")
	bool IsMissionPanelVisible() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Bond|UI")
	void OnPlayerWidgetsCreated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Bond|Mission UI")
	void OnMissionObjectiveUIInitialized(const TArray<UMissionObjective*>& ActiveObjectives);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bond|Mission UI")
	void OnMissionPanelVisibilityChanged(bool bVisible);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bond|UI")
	void UpdateDamageState(float CurrentHealth,float MaxHealth,float HealthPercent);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bond|UI")
	void PlayDamageFlash(float DamageAmount);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bond|UI")
	void SetRegenerating(bool bIsRegenerating);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bond|UI")
	void HandleBondDeath();

	UFUNCTION(BlueprintImplementableEvent, Category = "Bond|UI")
	void HandleBondWin();

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

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Bond|UI",
		meta = (AllowPrivateAccess = "true")
	)
	TSubclassOf<UUserWidget> DeathWidgetClass;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Bond|UI",
		meta = (AllowPrivateAccess = "true")
	)
	TSubclassOf<UUserWidget> WinWidgetClass;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Bond|UI",
		meta = (AllowPrivateAccess = "true")
	)
	TSubclassOf<UUserWidget> InteractionPromptWidgetClass;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Bond|UI",
		meta = (AllowPrivateAccess = "true")
	)
	FName InteractionPromptTextBlockName = TEXT("PromptText");

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Bond|UI",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.0")
	)
	float DeathWidgetDelay = 1.15f;

	UPROPERTY()
	TObjectPtr<UUserWidget> MainHUDWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> TimeSlowHUDWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> DeathWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> WinWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> InteractionPromptWidget;

	UPROPERTY()
	TObjectPtr<AJamesBondCharacter> PossessedBond;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bond|Mission UI", meta = (AllowPrivateAccess = "true"))
	bool bMissionPanelVisible = true;

	FTimerHandle DeferredWeaponBindingTimer;
	FTimerHandle DeathWidgetTimer;
	FTimerHandle DeferredMissionUIInitializationTimer;

	void CreatePlayerWidgets();
	void ShowDeathWidget();
	void RestoreGameplayInput();
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

	UFUNCTION()
	void HandleMissionObjectiveProgressChanged(FName ObjectiveId, int32 CurrentProgress, int32 RequiredProgress);

	UFUNCTION()
	void HandleMissionObjectiveCompleted(FName ObjectiveId);
};
