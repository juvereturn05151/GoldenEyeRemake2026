#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "SurveillanceCameraRotationComponent.generated.h"

class USceneComponent;

UCLASS(ClassGroup = (Surveillance), meta = (BlueprintSpawnableComponent))
class GOLDENEYEFINAL_API USurveillanceCameraRotationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USurveillanceCameraRotationComponent();

	UFUNCTION(BlueprintCallable, Category = "Surveillance Camera|Rotation")
	void StartScanning();

	UFUNCTION(BlueprintCallable, Category = "Surveillance Camera|Rotation")
	void StopScanning();

	UFUNCTION(BlueprintCallable, Category = "Surveillance Camera|Rotation")
	void SetTargetComponentByName(FName ComponentName);

	UFUNCTION(BlueprintCallable, Category = "Surveillance Camera|Rotation")
	void ResetToCenter();

	UFUNCTION(BlueprintPure, Category = "Surveillance Camera|Rotation")
	bool IsScanning() const;

	UFUNCTION(BlueprintPure, Category = "Surveillance Camera|Rotation")
	USceneComponent* GetTargetSceneComponent() const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surveillance Camera|Rotation")
	bool bStartScanningOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surveillance Camera|Rotation")
	bool bUseLocalRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surveillance Camera|Rotation", meta = (ClampMin = "0.0"))
	float LeftYawDegrees = -60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surveillance Camera|Rotation", meta = (ClampMin = "0.0"))
	float RightYawDegrees = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surveillance Camera|Rotation", meta = (ClampMin = "0.0"))
	float DegreesPerSecond = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surveillance Camera|Rotation", meta = (ClampMin = "0.0"))
	float PauseAtEndsSeconds = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surveillance Camera|Rotation", meta = (UseComponentPicker, AllowedClasses = "/Script/Engine.SceneComponent"))
	FComponentReference TargetComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surveillance Camera|Rotation")
	FName TargetComponentName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surveillance Camera|Rotation")
	bool bUseOwnerRootWhenTargetMissing = true;

private:
	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> CachedTargetComponent;

	FRotator BaseRelativeRotation = FRotator::ZeroRotator;
	FRotator BaseWorldRotation = FRotator::ZeroRotator;
	float CurrentYawOffset = 0.0f;
	float Direction = 1.0f;
	float PauseTimer = 0.0f;
	bool bIsScanning = false;

	USceneComponent* ResolveTargetComponent() const;
	void CacheTargetComponent();
	void CaptureBaseRotation();
	void ApplyYawOffset(float YawOffset);
	float GetLeftLimit() const;
	float GetRightLimit() const;
};
