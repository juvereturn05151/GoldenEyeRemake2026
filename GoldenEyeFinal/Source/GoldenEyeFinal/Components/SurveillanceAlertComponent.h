/*
Name: Ju-ve Chankasemporn
E-mail: juvereturn@gmail.com
@2026 MyLoyalFans Productions
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurveillanceAlertComponent.generated.h"

class ULightComponent;
class UAudioComponent;
class USoundBase;
class UEnemySpawnerComponent;

UCLASS(ClassGroup = (Surveillance), meta = (BlueprintSpawnableComponent))
class GOLDENEYEFINAL_API USurveillanceAlertComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USurveillanceAlertComponent();

	UFUNCTION(BlueprintCallable, Category = "Surveillance Camera|Alert")
	void SetAlertLight(ULightComponent* InAlertLight);

	UFUNCTION(BlueprintPure, Category = "Surveillance Camera|Alert")
	ULightComponent* GetAlertLight() const;

	UFUNCTION(BlueprintCallable, Category = "Surveillance Camera|Alert")
	void StartAlertFlash(float OverrideDuration = -1.0f);

	UFUNCTION(BlueprintCallable, Category = "Surveillance Camera|Alert")
	void StopAlertFlash();

	UFUNCTION(BlueprintPure, Category = "Surveillance Camera|Alert")
	bool IsAlertFlashing() const;

	UFUNCTION(BlueprintCallable, Category = "Surveillance Camera|Alert")
	void BindLinkedEnemySpawners();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surveillance Camera|Alert", meta = (AllowPrivateAccess = "true", UseComponentPicker, AllowedClasses = "/Script/Engine.LightComponent"))
	TObjectPtr<ULightComponent> AlertLight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surveillance Camera|Alert", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float FlashDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surveillance Camera|Alert", meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
	float FlashInterval = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surveillance Camera|Alert", meta = (AllowPrivateAccess = "true"))
	FLinearColor AlertColor = FLinearColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surveillance Camera|Alert", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float AlertIntensity = 8000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surveillance Camera|Alert|Audio", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> AlertSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surveillance Camera|Alert|Audio", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float AlertSoundVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surveillance Camera|Alert|Audio", meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
	float AlertSoundPitch = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surveillance Camera|Alert|Audio", meta = (AllowPrivateAccess = "true"))
	bool bPlayAlertSound2D = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surveillance Camera|Alert", meta = (AllowPrivateAccess = "true"))
	bool bHideAlertLightOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surveillance Camera|Alert|Spawner", meta = (AllowPrivateAccess = "true", UseComponentPicker, AllowedClasses = "/Script/GoldenEyeFinal.EnemySpawnerComponent"))
	TArray<TObjectPtr<UEnemySpawnerComponent>> LinkedEnemySpawners;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surveillance Camera|Alert|Spawner", meta = (AllowPrivateAccess = "true"))
	bool bBindLinkedSpawnersOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surveillance Camera|Alert|Spawner", meta = (AllowPrivateAccess = "true"))
	bool bDestroyLinkedSpawnersOnCameraDestroyed = true;

	FTimerHandle FlashToggleTimer;
	FTimerHandle FlashStopTimer;
	UPROPERTY()
	TObjectPtr<UAudioComponent> ActiveAlertAudioComponent;

	bool bIsFlashing = false;
	bool bOriginalVisibility = false;
	FLinearColor OriginalLightColor = FLinearColor::White;
	float OriginalIntensity = 0.0f;

	ULightComponent* ResolveAlertLight() const;
	void UnbindLinkedEnemySpawners();
	void DestroyLinkedEnemySpawners();
	void ToggleAlertLight();
	void ApplyAlertLightState(bool bLightOn);
	void PlayAlertSound();
	void StopAlertSound();
	void CacheOriginalLightState();
	void RestoreOriginalLightState();

	UFUNCTION()
	void HandleLinkedSpawnerTriggered(AActor* BondActor);
};
