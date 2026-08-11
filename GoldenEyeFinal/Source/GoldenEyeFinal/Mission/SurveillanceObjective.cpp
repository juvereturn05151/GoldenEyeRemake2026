#include "SurveillanceObjective.h"

#include "GameplayMissionSubsystem.h"

void USurveillanceObjective::InitializeSurveillanceObjective(
	FName InObjectiveId,
	FText InDisplayName,
	FText InDescription,
	FName InTargetGroupId
)
{
	TargetGroupId = InTargetGroupId;
	InitializeObjective(InObjectiveId, InDisplayName, InDescription, 1);
}

void USurveillanceObjective::ActivateObjective()
{
	if (TargetGroupId == NAME_None)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission] Empty group ID for surveillance objective=%s"), *ObjectiveId.ToString());
	}

	if (bAutomaticallyCountRegisteredCameras)
	{
		RefreshRequiredProgressFromRegisteredActors();
		RebuildProgressFromCompletedRegisteredActors();
	}

	Super::ActivateObjective();

	if (CurrentProgress >= RequiredProgress)
	{
		CompleteObjective();
	}
}

void USurveillanceObjective::HandleMissionEvent(const FMissionEventData& EventData)
{
	if (Status == EObjectiveStatus::Completed || Status == EObjectiveStatus::Failed)
	{
		return;
	}

	if (EventData.EventTag == TEXT("Camera.Registered") && bAutomaticallyCountRegisteredCameras)
	{
		RefreshRequiredProgressFromRegisteredActors();
		OnObjectiveProgressChanged.Broadcast(ObjectiveId, CurrentProgress, RequiredProgress);
		return;
	}

	if (!ShouldHandleEvent(EventData))
	{
		return;
	}

	AActor* DestroyedActor = EventData.Target;

	if (!DestroyedActor)
	{
		return;
	}

	const FObjectKey ActorKey(DestroyedActor);

	if (CountedDestroyedActors.Contains(ActorKey))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Mission] Duplicate destruction ignored=%s"), *GetNameSafe(DestroyedActor));
		return;
	}

	CountedDestroyedActors.Add(ActorKey);

	SetProgress(CurrentProgress + FMath::Max(1, EventData.Amount), RequiredProgress);

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[Mission] Surveillance camera counted Objective=%s Camera=%s Progress=%d/%d"),
		*ObjectiveId.ToString(),
		*GetNameSafe(DestroyedActor),
		CurrentProgress,
		RequiredProgress
	);

	if (CurrentProgress >= RequiredProgress)
	{
		CompleteObjective();
	}
}

void USurveillanceObjective::RefreshRequiredProgressFromRegisteredActors()
{
	const UWorld* World = GetWorld();

	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission] Missing world while counting surveillance actors"));
		return;
	}

	const UGameplayMissionSubsystem* MissionSubsystem = World->GetSubsystem<UGameplayMissionSubsystem>();

	if (!MissionSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission] Missing subsystem while counting surveillance actors"));
		return;
	}

	const int32 RegisteredCount = MissionSubsystem->CountRegisteredActorsForGroup(TargetGroupId);

	if (RegisteredCount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Mission] No registered cameras Group=%s"), *TargetGroupId.ToString());
	}

	RequiredProgress = FMath::Max(1, RegisteredCount);

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[Mission] Surveillance objective count Objective=%s Group=%s Cameras=%d Required=%d"),
		*ObjectiveId.ToString(),
		*TargetGroupId.ToString(),
		RegisteredCount,
		RequiredProgress
	);
}

void USurveillanceObjective::RebuildProgressFromCompletedRegisteredActors()
{
	const UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	const UGameplayMissionSubsystem* MissionSubsystem = World->GetSubsystem<UGameplayMissionSubsystem>();

	if (!MissionSubsystem)
	{
		return;
	}

	CountedDestroyedActors.Reset();

	int32 CompletedCount = 0;
	const TArray<TWeakObjectPtr<AActor>> RegisteredActors = MissionSubsystem->GetRegisteredActorsForGroup(TargetGroupId);

	for (const TWeakObjectPtr<AActor>& RegisteredActor : RegisteredActors)
	{
		AActor* Actor = RegisteredActor.Get();

		if (Actor && MissionSubsystem->IsRegisteredActorCompleted(Actor))
		{
			CountedDestroyedActors.Add(FObjectKey(Actor));
			++CompletedCount;
		}
	}

	CurrentProgress = FMath::Clamp(CompletedCount, 0, RequiredProgress);

	if (CompletedCount > 0)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[Mission] Surveillance objective restored progress Objective=%s Progress=%d/%d"),
			*ObjectiveId.ToString(),
			CurrentProgress,
			RequiredProgress
		);
	}
}

bool USurveillanceObjective::ShouldHandleEvent(const FMissionEventData& EventData) const
{
	return
		EventData.EventTag == TEXT("Camera.Destroyed") &&
		EventData.ContextId == TargetGroupId;
}
