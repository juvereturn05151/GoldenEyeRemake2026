#include "GameplayMissionSubsystem.h"

#include "MissionObjective.h"
#include "MissionRelevantActor.h"
#include "SurveillanceObjective.h"

bool UGameplayMissionSubsystem::RegisterMissionActor(AActor* MissionActor)
{
	if (!MissionActor)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission] RegisterMissionActor failed: actor is null"));
		return false;
	}

	if (!MissionActor->GetClass()->ImplementsInterface(UMissionRelevantActor::StaticClass()))
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission] RegisterMissionActor failed: actor=%s does not implement MissionRelevantActor"), *GetNameSafe(MissionActor));
		return false;
	}

	const FName GroupId = IMissionRelevantActor::Execute_GetMissionGroupId(MissionActor);
	const FName ActorId = IMissionRelevantActor::Execute_GetMissionActorId(MissionActor);
	const bool bCompleted = IMissionRelevantActor::Execute_IsMissionActorCompleted(MissionActor);

	return RegisterMissionActorWithIds(MissionActor, GroupId, ActorId, bCompleted);
}

bool UGameplayMissionSubsystem::RegisterMissionActorWithIds(AActor* MissionActor, FName GroupId, FName ActorId, bool bIsCompleted)
{
	if (!MissionActor)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission] RegisterMissionActorWithIds failed: actor is null"));
		return false;
	}

	if (GroupId == NAME_None)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission] Empty group ID actor=%s"), *GetNameSafe(MissionActor));
		return false;
	}

	if (ActorId == NAME_None)
	{
		ActorId = MakeFallbackActorId(MissionActor);
		UE_LOG(LogTemp, Log, TEXT("[Mission] Optional actor ID not set actor=%s UsingRuntimeId=%s"), *GetNameSafe(MissionActor), *ActorId.ToString());
	}

	const FObjectKey ActorKey(MissionActor);

	if (const FObjectKey* ExistingActorKey = RegisteredActorIds.Find(ActorId))
	{
		if (*ExistingActorKey != ActorKey)
		{
			UE_LOG(LogTemp, Error, TEXT("[Mission] Duplicate actor ID=%s ExistingActor=%s NewActor=%s"), *ActorId.ToString(), *GetNameSafe(ExistingActorKey->ResolveObjectPtr()), *GetNameSafe(MissionActor));
			return false;
		}
	}

	FMissionRegisteredActor& RegisteredActor = RegisteredActors.FindOrAdd(ActorKey);
	RegisteredActor.Actor = MissionActor;
	RegisteredActor.GroupId = GroupId;
	RegisteredActor.ActorId = ActorId;
	RegisteredActor.bCompleted = bIsCompleted;
	RegisteredActorIds.Add(ActorId, ActorKey);

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[Mission] Registered actor=%s Group=%s Id=%s GroupCount=%d"),
		*GetNameSafe(MissionActor),
		*GroupId.ToString(),
		*ActorId.ToString(),
		CountRegisteredActorsForGroup(GroupId)
	);

	return true;
}

void UGameplayMissionSubsystem::UnregisterMissionActor(AActor* MissionActor)
{
	if (!MissionActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Mission] UnregisterMissionActor ignored: actor is null"));
		return;
	}

	const FObjectKey ActorKey(MissionActor);

	if (FMissionRegisteredActor* RegisteredActor = RegisteredActors.Find(ActorKey))
	{
		RegisteredActorIds.Remove(RegisteredActor->ActorId);
		RegisteredActors.Remove(ActorKey);
		UE_LOG(LogTemp, Log, TEXT("[Mission] Unregistered actor=%s"), *GetNameSafe(MissionActor));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Mission] Unregister requested for unregistered actor=%s"), *GetNameSafe(MissionActor));
}

void UGameplayMissionSubsystem::BroadcastMissionEvent(const FMissionEventData& EventData)
{
	UE_LOG(
		LogTemp,
		Log,
		TEXT("[Mission] Event=%s Target=%s Group=%s"),
		*EventData.EventTag.ToString(),
		*GetNameSafe(EventData.Target),
		*EventData.ContextId.ToString()
	);

	if (EventData.EventTag == TEXT("Camera.Destroyed"))
	{
		MarkRegisteredActorCompleted(EventData.Target);
	}

	OnMissionEventReceived.Broadcast(EventData);

	for (UMissionObjective* Objective : ActiveObjectives)
	{
		if (Objective)
		{
			Objective->HandleMissionEvent(EventData);
		}
	}
}

void UGameplayMissionSubsystem::StartMission()
{
	for (UMissionObjective* Objective : ActiveObjectives)
	{
		if (Objective && Objective->GetStatus() == EObjectiveStatus::Inactive)
		{
			Objective->ActivateObjective();
		}
	}
}

USurveillanceObjective* UGameplayMissionSubsystem::StartSurveillanceObjective(FName ObjectiveId, FText DisplayName, FName TargetGroupId)
{
	if (TargetGroupId == NAME_None)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission] Empty group ID objective=%s"), *ObjectiveId.ToString());
		return nullptr;
	}

	if (UMissionObjective* ExistingObjective = GetObjective(ObjectiveId))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Mission] Objective already exists=%s"), *ObjectiveId.ToString());
		return Cast<USurveillanceObjective>(ExistingObjective);
	}

	USurveillanceObjective* Objective = NewObject<USurveillanceObjective>(this);

	if (!Objective)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission] Failed to create surveillance objective=%s"), *ObjectiveId.ToString());
		return nullptr;
	}

	Objective->InitializeSurveillanceObjective(ObjectiveId, DisplayName, TargetGroupId);
	Objective->OnObjectiveProgressChanged.AddDynamic(this, &UGameplayMissionSubsystem::HandleObjectiveProgressChanged);
	Objective->OnObjectiveCompleted.AddDynamic(this, &UGameplayMissionSubsystem::HandleObjectiveCompleted);

	ActiveObjectives.Add(Objective);
	Objective->ActivateObjective();

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[Mission] Started objective=%s Group=%s Required=%d"),
		*ObjectiveId.ToString(),
		*TargetGroupId.ToString(),
		Objective->GetRequiredProgress()
	);

	return Objective;
}

bool UGameplayMissionSubsystem::IsObjectiveCompleted(FName ObjectiveId) const
{
	const UMissionObjective* Objective = GetObjective(ObjectiveId);
	return Objective && Objective->GetStatus() == EObjectiveStatus::Completed;
}

void UGameplayMissionSubsystem::GetObjectiveProgress(FName ObjectiveId, int32& CurrentProgress, int32& RequiredProgress) const
{
	CurrentProgress = 0;
	RequiredProgress = 0;

	const UMissionObjective* Objective = GetObjective(ObjectiveId);

	if (!Objective)
	{
		return;
	}

	CurrentProgress = Objective->GetCurrentProgress();
	RequiredProgress = Objective->GetRequiredProgress();
}

UMissionObjective* UGameplayMissionSubsystem::GetObjective(FName ObjectiveId) const
{
	for (UMissionObjective* Objective : ActiveObjectives)
	{
		if (Objective && Objective->GetObjectiveId() == ObjectiveId)
		{
			return Objective;
		}
	}

	return nullptr;
}

int32 UGameplayMissionSubsystem::CountRegisteredActorsForGroup(FName GroupId) const
{
	int32 Count = 0;

	for (const TPair<FObjectKey, FMissionRegisteredActor>& RegisteredActorPair : RegisteredActors)
	{
		const FMissionRegisteredActor& RegisteredActor = RegisteredActorPair.Value;

		if (RegisteredActor.GroupId == GroupId && RegisteredActor.Actor.IsValid())
		{
			++Count;
		}
	}

	return Count;
}

bool UGameplayMissionSubsystem::IsRegisteredActorCompleted(AActor* MissionActor) const
{
	if (!MissionActor)
	{
		return false;
	}

	const FMissionRegisteredActor* RegisteredActor = RegisteredActors.Find(FObjectKey(MissionActor));
	return RegisteredActor && RegisteredActor->bCompleted;
}

TArray<TWeakObjectPtr<AActor>> UGameplayMissionSubsystem::GetRegisteredActorsForGroup(FName GroupId) const
{
	TArray<TWeakObjectPtr<AActor>> MatchingActors;

	for (const TPair<FObjectKey, FMissionRegisteredActor>& RegisteredActorPair : RegisteredActors)
	{
		const FMissionRegisteredActor& RegisteredActor = RegisteredActorPair.Value;

		if (RegisteredActor.GroupId == GroupId && RegisteredActor.Actor.IsValid())
		{
			MatchingActors.Add(RegisteredActor.Actor);
		}
	}

	return MatchingActors;
}

void UGameplayMissionSubsystem::HandleObjectiveProgressChanged(FName ObjectiveId, int32 CurrentProgress, int32 RequiredProgress)
{
	UE_LOG(LogTemp, Log, TEXT("[Mission] Objective progress Objective=%s Progress=%d/%d"), *ObjectiveId.ToString(), CurrentProgress, RequiredProgress);
	OnObjectiveProgressChanged.Broadcast(ObjectiveId, CurrentProgress, RequiredProgress);
}

void UGameplayMissionSubsystem::HandleObjectiveCompleted(FName ObjectiveId)
{
	UE_LOG(LogTemp, Log, TEXT("[Mission] Objective completed=%s"), *ObjectiveId.ToString());
	OnObjectiveCompleted.Broadcast(ObjectiveId);
}

FName UGameplayMissionSubsystem::MakeFallbackActorId(AActor* MissionActor) const
{
	if (!MissionActor)
	{
		return NAME_None;
	}

	return MissionActor->GetFName();
}

void UGameplayMissionSubsystem::MarkRegisteredActorCompleted(AActor* MissionActor)
{
	if (!MissionActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Mission] Destruction notification from an unregistered actor Target=null"));
		return;
	}

	FMissionRegisteredActor* RegisteredActor = RegisteredActors.Find(FObjectKey(MissionActor));

	if (!RegisteredActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Mission] Destruction notification from an unregistered actor=%s"), *GetNameSafe(MissionActor));
		return;
	}

	RegisteredActor->bCompleted = true;
	UE_LOG(
		LogTemp,
		Log,
		TEXT("[Mission] Marked actor complete=%s Group=%s"),
		*GetNameSafe(MissionActor),
		*RegisteredActor->GroupId.ToString()
	);
}
