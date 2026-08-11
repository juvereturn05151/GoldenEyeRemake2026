#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GoldenEyeMissionInitializer.generated.h"

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	bool bInitializeOnBeginPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	TArray<FGoldenEyeSurveillanceObjectiveDefinition> SurveillanceObjectives;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	TArray<FGoldenEyeEventObjectiveDefinition> EventObjectives;

protected:
	virtual void BeginPlay() override;
};
