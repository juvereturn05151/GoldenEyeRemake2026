#include "EventMissionObjective.h"

void UEventMissionObjective::InitializeEventObjective(
	FName InObjectiveId,
	FText InDisplayName,
	FText InDescription,
	FName InRequiredEventTag,
	FName InRequiredContextId,
	int32 InRequiredProgress
)
{
	RequiredEventTag = InRequiredEventTag;
	RequiredContextId = InRequiredContextId;
	InitializeObjective(InObjectiveId, InDisplayName, InDescription, InRequiredProgress);
}

void UEventMissionObjective::HandleMissionEvent(const FMissionEventData& EventData)
{
	if (Status == EObjectiveStatus::Completed || Status == EObjectiveStatus::Failed)
	{
		return;
	}

	if (RequiredEventTag != NAME_None && EventData.EventTag != RequiredEventTag)
	{
		return;
	}

	if (RequiredContextId != NAME_None && EventData.ContextId != RequiredContextId)
	{
		return;
	}

	SetProgress(CurrentProgress + FMath::Max(1, EventData.Amount), RequiredProgress);

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[Mission] Event objective counted Objective=%s Event=%s Context=%s Progress=%d/%d"),
		*ObjectiveId.ToString(),
		*EventData.EventTag.ToString(),
		*EventData.ContextId.ToString(),
		CurrentProgress,
		RequiredProgress
	);

	if (CurrentProgress >= RequiredProgress)
	{
		CompleteObjective();
	}
}
