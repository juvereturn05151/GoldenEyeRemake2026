#pragma once

#include "CoreMinimal.h"
#include "MissionTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayMissionSubsystem.generated.h"

class UMissionObjective;
class UEventMissionObjective;
class USurveillanceObjective;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FMissionProgressChangedSignature,
	FName,
	ObjectiveId,
	int32,
	CurrentProgress,
	int32,
	RequiredProgress
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FMissionObjectiveCompletedSignature,
	FName,
	ObjectiveId
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FMissionEventReceivedSignature,
	const FMissionEventData&,
	EventData
);

struct FMissionRegisteredActor
{
	TWeakObjectPtr<AActor> Actor;
	FName GroupId = NAME_None;
	FName ActorId = NAME_None;
	bool bCompleted = false;
};

UCLASS()
class GOLDENEYEFINAL_API UGameplayMissionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Mission")
	bool RegisterMissionActor(AActor* MissionActor);

	UFUNCTION(BlueprintCallable, Category = "Mission")
	bool RegisterMissionActorWithIds(AActor* MissionActor, FName GroupId, FName ActorId, bool bIsCompleted);

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void UnregisterMissionActor(AActor* MissionActor);

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void BroadcastMissionEvent(const FMissionEventData& EventData);

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void StartMission();

	UFUNCTION(BlueprintCallable, Category = "Mission")
	USurveillanceObjective* StartSurveillanceObjective(
		FName ObjectiveId,
		FText DisplayName,
		FText Description,
		FName TargetGroupId
	);

	UFUNCTION(BlueprintCallable, Category = "Mission")
	UEventMissionObjective* StartEventObjective(
		FName ObjectiveId,
		FText DisplayName,
		FText Description,
		FName RequiredEventTag,
		FName RequiredContextId,
		int32 RequiredProgress = 1
	);

	UFUNCTION(BlueprintPure, Category = "Mission")
	bool IsObjectiveCompleted(FName ObjectiveId) const;

	UFUNCTION(BlueprintPure, Category = "Mission")
	void GetObjectiveProgress(FName ObjectiveId, int32& CurrentProgress, int32& RequiredProgress) const;

	UFUNCTION(BlueprintPure, Category = "Mission")
	UMissionObjective* GetObjective(FName ObjectiveId) const;

	UFUNCTION(BlueprintPure, Category = "Mission")
	const TArray<UMissionObjective*>& GetActiveObjectives() const;

	int32 CountRegisteredActorsForGroup(FName GroupId) const;
	bool IsRegisteredActorCompleted(AActor* MissionActor) const;
	TArray<TWeakObjectPtr<AActor>> GetRegisteredActorsForGroup(FName GroupId) const;

	UPROPERTY(BlueprintAssignable, Category = "Mission")
	FMissionProgressChangedSignature OnObjectiveProgressChanged;

	UPROPERTY(BlueprintAssignable, Category = "Mission")
	FMissionObjectiveCompletedSignature OnObjectiveCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Mission")
	FMissionEventReceivedSignature OnMissionEventReceived;

private:
	UFUNCTION()
	void HandleObjectiveProgressChanged(FName ObjectiveId, int32 CurrentProgress, int32 RequiredProgress);

	UFUNCTION()
	void HandleObjectiveCompleted(FName ObjectiveId);

	FName MakeFallbackActorId(AActor* MissionActor) const;
	void MarkRegisteredActorCompleted(AActor* MissionActor);

	TMap<FObjectKey, FMissionRegisteredActor> RegisteredActors;
	TMap<FName, FObjectKey> RegisteredActorIds;

	UPROPERTY()
	TArray<TObjectPtr<UMissionObjective>> ActiveObjectives;
};
