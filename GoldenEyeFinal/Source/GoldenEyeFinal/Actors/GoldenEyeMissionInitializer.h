#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GoldenEyeMissionInitializer.generated.h"

class AAutomaticDoorActor;
class UEnemySpawnerComponent;
class USceneComponent;

USTRUCT(BlueprintType)
struct FGoldenEyeEventObjectiveDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	FName ObjectiveId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission", meta = (MultiLine = "true"))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	FName RequiredEventTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	FName RequiredContextId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission", meta = (ClampMin = "1"))
	int32 RequiredProgress = 1;
};

USTRUCT(BlueprintType)
struct FGoldenEyeSurveillanceObjectiveDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	FName ObjectiveId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission", meta = (MultiLine = "true"))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	FName TargetGroupId = NAME_None;
};

UCLASS()
class GOLDENEYEFINAL_API AGoldenEyeMissionInitializer : public AActor
{
	GENERATED_BODY()

public:
	AGoldenEyeMissionInitializer();

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void InitializeMissionAndUI();

	UFUNCTION(BlueprintCallable, Category = "Mission")
	bool AreAllObjectivesCompleted() const;

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void CheckAllObjectivesCompleted();

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void HandleAllObjectivesCompleted();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	bool bInitializeOnBeginPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	TArray<FGoldenEyeSurveillanceObjectiveDefinition> SurveillanceObjectives;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	TArray<FGoldenEyeEventObjectiveDefinition> EventObjectives;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission|Final Wave")
	TObjectPtr<UEnemySpawnerComponent> FinalWaveSpawner;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission|Completion")
	TArray<TObjectPtr<AAutomaticDoorActor>> DoorsToUnlockOnAllObjectivesCompleted;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission|Completion")
	bool bUnlockDoorsOnAllObjectivesCompleted = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission|Completion")
	bool bSpawnFinalWaveOnAllObjectivesCompleted = true;

	UFUNCTION(BlueprintImplementableEvent, Category = "Mission|Completion")
	void OnAllObjectivesCompleted();

private:
	UFUNCTION()
	void HandleObjectiveCompleted(FName ObjectiveId);

	void BindMissionCompletionDelegate();

	bool bAllObjectivesCompletedHandled = false;
};
