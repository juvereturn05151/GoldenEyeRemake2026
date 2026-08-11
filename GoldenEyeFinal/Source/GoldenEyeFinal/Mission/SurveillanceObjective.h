#pragma once

#include "CoreMinimal.h"
#include "MissionObjective.h"
#include "SurveillanceObjective.generated.h"

UCLASS(Blueprintable)
class GOLDENEYEFINAL_API USurveillanceObjective : public UMissionObjective
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Mission|Objective")
	void InitializeSurveillanceObjective(
		FName InObjectiveId,
		FText InDisplayName,
		FText InDescription,
		FName InTargetGroupId
	);

	virtual void ActivateObjective() override;
	virtual void HandleMissionEvent(const FMissionEventData& EventData) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission|Surveillance")
	FName TargetGroupId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission|Surveillance")
	bool bAutomaticallyCountRegisteredCameras = true;

private:
	void RefreshRequiredProgressFromRegisteredActors();
	void RebuildProgressFromCompletedRegisteredActors();
	bool ShouldHandleEvent(const FMissionEventData& EventData) const;

	TSet<FObjectKey> CountedDestroyedActors;
};
