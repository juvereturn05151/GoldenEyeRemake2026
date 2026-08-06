#pragma once

#include "CoreMinimal.h"
#include "MissionTypes.generated.h"

UENUM(BlueprintType)
enum class EObjectiveStatus : uint8
{
	Inactive,
	Active,
	Completed,
	Failed
};

USTRUCT(BlueprintType)
struct GOLDENEYEFINAL_API FMissionEventData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FName EventTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	TObjectPtr<AActor> Instigator = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	TObjectPtr<AActor> Target = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FName ContextId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	int32 Amount = 1;
};
