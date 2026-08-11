#pragma once

#include "CoreMinimal.h"
#include "MissionObjective.h"
#include "EventMissionObjective.generated.h"

UCLASS(Blueprintable)
class GOLDENEYEFINAL_API UEventMissionObjective : public UMissionObjective
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Mission|Objective")
	void InitializeEventObjective(
		FName InObjectiveId,
		FText InDisplayName,
		FText InDescription,
		FName InRequiredEventTag,
		FName InRequiredContextId,
		int32 InRequiredProgress
	);

	virtual void HandleMissionEvent(const FMissionEventData& EventData) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission|Event")
	FName RequiredEventTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission|Event")
	FName RequiredContextId = NAME_None;
};
