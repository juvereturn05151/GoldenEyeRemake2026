/*
Name: Ju-ve Chankasemporn
E-mail: juvereturn@gmail.com
@2026 MyLoyalFans Productions
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BondTimeSlowComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FBondTimeSlowMeterChangedSignature,
	float,
	CurrentMeter,
	float,
	MaxMeter
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FBondTimeSlowStateChangedSignature,
	bool,
	bIsActive
);

UCLASS(ClassGroup = (Bond), meta = (BlueprintSpawnableComponent))
class GOLDENEYEFINAL_API UBondTimeSlowComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBondTimeSlowComponent();

	UFUNCTION(BlueprintCallable, Category = "Bond|Time Slow")
	void StartTimeSlow();

	UFUNCTION(BlueprintCallable, Category = "Bond|Time Slow")
	void StopTimeSlow();

	UFUNCTION(BlueprintPure, Category = "Bond|Time Slow")
	bool CanStartTimeSlow() const;

	UFUNCTION(BlueprintPure, Category = "Bond|Time Slow")
	bool IsTimeSlowActive() const;

	UFUNCTION(BlueprintPure, Category = "Bond|Time Slow")
	float GetCurrentMeter() const;

	UFUNCTION(BlueprintPure, Category = "Bond|Time Slow")
	float GetMaxMeter() const;

	UFUNCTION(BlueprintPure, Category = "Bond|Time Slow")
	float GetMeterPercent() const;

	UPROPERTY(BlueprintAssignable, Category = "Bond|Time Slow")
	FBondTimeSlowMeterChangedSignature OnMeterChanged;

	UPROPERTY(BlueprintAssignable, Category = "Bond|Time Slow")
	FBondTimeSlowStateChangedSignature OnTimeSlowStateChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Time Slow", meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float MaxMeter = 100.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bond|Time Slow", meta = (AllowPrivateAccess = "true"))
	float CurrentMeter = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Time Slow", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float DrainPerRealSecond = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Time Slow", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float RegenerationPerRealSecond = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Time Slow", meta = (AllowPrivateAccess = "true", ClampMin = "0.01", ClampMax = "1.0"))
	float TimeDilation = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Time Slow", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float RegenerationDelay = 1.0f;

	bool bIsActive = false;
	float InactiveRealSeconds = 0.0f;
	float LastRealTimeSeconds = 0.0f;

	void DrainMeter(float RealDeltaSeconds);
	void RegenerateMeter(float RealDeltaSeconds);
	void SetCurrentMeter(float NewMeter);
	void ApplyTimeDilation();
	void ClearTimeDilation();
	void BroadcastMeterChanged();
};
