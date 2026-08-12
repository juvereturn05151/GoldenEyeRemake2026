#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "BorisAIController.generated.h"

class ABorisCharacter;
class AJamesBondCharacter;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;

UCLASS()
class GOLDENEYEFINAL_API ABorisAIController : public AAIController
{
	GENERATED_BODY()

public:
	ABorisAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boris|AI")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boris|AI|Perception", meta = (ClampMin = "0.0"))
	float SightRadius = 1600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boris|AI|Perception", meta = (ClampMin = "0.0"))
	float LoseSightRadius = 2000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boris|AI|Perception", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float PeripheralVisionHalfAngleDegrees = 75.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boris|AI|Perception", meta = (ClampMin = "0.0"))
	float SightMaxAge = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boris|Debug")
	bool bDebugPerception = true;

private:
	UFUNCTION()
	void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	void ConfigurePerception();
	void StartSightPolling();
	void StopSightPolling();
	void CheckBondSight();
	bool HasSightToBond(AJamesBondCharacter* Bond) const;
	bool IsBond(AActor* Actor) const;

	UPROPERTY()
	TObjectPtr<ABorisCharacter> ControlledBoris;

	FTimerHandle SightPollingTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Boris|AI|Perception", meta = (AllowPrivateAccess = "true", ClampMin = "0.05"))
	float SightPollingInterval = 0.15f;

	UPROPERTY(EditDefaultsOnly, Category = "Boris|AI|Perception", meta = (AllowPrivateAccess = "true"))
	bool bUseSightPollingBackup = true;
};
