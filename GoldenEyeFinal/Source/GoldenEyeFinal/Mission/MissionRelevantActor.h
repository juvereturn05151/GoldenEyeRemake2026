#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MissionRelevantActor.generated.h"

UINTERFACE(Blueprintable)
class GOLDENEYEFINAL_API UMissionRelevantActor : public UInterface
{
	GENERATED_BODY()
};

class GOLDENEYEFINAL_API IMissionRelevantActor
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Mission")
	FName GetMissionGroupId() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Mission")
	FName GetMissionActorId() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Mission")
	bool IsMissionActorCompleted() const;
};
