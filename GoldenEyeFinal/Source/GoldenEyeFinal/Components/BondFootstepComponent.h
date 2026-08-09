/*
Name: Ju-ve Chankasemporn
E-mail: juvereturn@gmail.com
@2026 MyLoyalFans Productions
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BondFootstepComponent.generated.h"

class USoundBase;

UCLASS(ClassGroup = (Bond), meta = (BlueprintSpawnableComponent))
class GOLDENEYEFINAL_API UBondFootstepComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBondFootstepComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Footsteps", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> FootstepSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Footsteps", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MinimumSpeed = 80.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Footsteps", meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
	float WalkStepInterval = 0.45f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Footsteps", meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
	float RunStepInterval = 0.32f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Footsteps", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float RunSpeedThreshold = 450.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Footsteps", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float FootstepVolume = 0.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bond|Footsteps", meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
	float FootstepPitch = 1.0f;

	float StepTimer = 0.0f;

	bool ShouldPlayFootsteps(float& OutSpeed) const;
	float GetCurrentStepInterval(float Speed) const;
	void PlayFootstep();
};
