#pragma once

#include "CoreMinimal.h"
#include "MissionTypes.h"
#include "UObject/Object.h"
#include "MissionObjective.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FMissionObjectiveProgressChangedSignature,
	FName,
	ObjectiveId,
	int32,
	CurrentProgress,
	int32,
	RequiredProgress
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FMissionObjectiveStatusSignature,
	FName,
	ObjectiveId
);

UCLASS(Blueprintable)
class GOLDENEYEFINAL_API UMissionObjective : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Mission|Objective")
	virtual void InitializeObjective(FName InObjectiveId, FText InDisplayName, FText InDescription, int32 InRequiredProgress);

	UFUNCTION(BlueprintCallable, Category = "Mission|Objective")
	virtual void ActivateObjective();

	UFUNCTION(BlueprintCallable, Category = "Mission|Objective")
	virtual void HandleMissionEvent(const FMissionEventData& EventData);

	UFUNCTION(BlueprintCallable, Category = "Mission|Objective")
	virtual void CompleteObjective();

	UFUNCTION(BlueprintCallable, Category = "Mission|Objective")
	virtual void FailObjective();

	UFUNCTION(BlueprintPure, Category = "Mission|Objective")
	virtual FText GetProgressText() const;

	UFUNCTION(BlueprintPure, Category = "Mission|Objective")
	FText GetDisplayName() const;

	UFUNCTION(BlueprintPure, Category = "Mission|Objective")
	FText GetDescription() const;

	UFUNCTION(BlueprintPure, Category = "Mission|Objective")
	FName GetObjectiveId() const;

	UFUNCTION(BlueprintPure, Category = "Mission|Objective")
	EObjectiveStatus GetStatus() const;

	UFUNCTION(BlueprintPure, Category = "Mission|Objective")
	int32 GetCurrentProgress() const;

	UFUNCTION(BlueprintPure, Category = "Mission|Objective")
	int32 GetRequiredProgress() const;

	UPROPERTY(BlueprintAssignable, Category = "Mission|Objective")
	FMissionObjectiveProgressChangedSignature OnObjectiveProgressChanged;

	UPROPERTY(BlueprintAssignable, Category = "Mission|Objective")
	FMissionObjectiveStatusSignature OnObjectiveCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Mission|Objective")
	FMissionObjectiveStatusSignature OnObjectiveFailed;

protected:
	void SetProgress(int32 NewCurrentProgress, int32 NewRequiredProgress);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission|Objective")
	FName ObjectiveId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission|Objective")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission|Objective")
	FText Description;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Mission|Objective")
	EObjectiveStatus Status = EObjectiveStatus::Inactive;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Mission|Objective")
	int32 CurrentProgress = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Mission|Objective")
	int32 RequiredProgress = 1;
};
