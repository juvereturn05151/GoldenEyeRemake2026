#include "MissionObjective.h"

void UMissionObjective::InitializeObjective(
	FName InObjectiveId,
	FText InDisplayName,
	FText InDescription,
	int32 InRequiredProgress
)
{
	ObjectiveId = InObjectiveId;
	DisplayName = InDisplayName;
	Description = InDescription;
	Status = EObjectiveStatus::Inactive;
	CurrentProgress = 0;
	RequiredProgress = FMath::Max(1, InRequiredProgress);
}

void UMissionObjective::ActivateObjective()
{
	if (Status == EObjectiveStatus::Completed || Status == EObjectiveStatus::Failed)
	{
		return;
	}

	Status = EObjectiveStatus::Active;
	OnObjectiveProgressChanged.Broadcast(ObjectiveId, CurrentProgress, RequiredProgress);
}

void UMissionObjective::HandleMissionEvent(const FMissionEventData& EventData)
{
}

void UMissionObjective::CompleteObjective()
{
	if (Status == EObjectiveStatus::Completed || Status == EObjectiveStatus::Failed)
	{
		return;
	}

	Status = EObjectiveStatus::Completed;
	OnObjectiveProgressChanged.Broadcast(ObjectiveId, CurrentProgress, RequiredProgress);
	OnObjectiveCompleted.Broadcast(ObjectiveId);
}

void UMissionObjective::FailObjective()
{
	if (Status == EObjectiveStatus::Completed || Status == EObjectiveStatus::Failed)
	{
		return;
	}

	Status = EObjectiveStatus::Failed;
	OnObjectiveFailed.Broadcast(ObjectiveId);
}

FText UMissionObjective::GetProgressText() const
{
	return FText::Format(
		NSLOCTEXT("MissionObjective", "ProgressFormat", "{0}/{1}"),
		FText::AsNumber(CurrentProgress),
		FText::AsNumber(RequiredProgress)
	);
}

FName UMissionObjective::GetObjectiveId() const
{
	return ObjectiveId;
}

EObjectiveStatus UMissionObjective::GetStatus() const
{
	return Status;
}

int32 UMissionObjective::GetCurrentProgress() const
{
	return CurrentProgress;
}

int32 UMissionObjective::GetRequiredProgress() const
{
	return RequiredProgress;
}

void UMissionObjective::SetProgress(int32 NewCurrentProgress, int32 NewRequiredProgress)
{
	RequiredProgress = FMath::Max(1, NewRequiredProgress);
	CurrentProgress = FMath::Clamp(NewCurrentProgress, 0, RequiredProgress);
	OnObjectiveProgressChanged.Broadcast(ObjectiveId, CurrentProgress, RequiredProgress);
}
