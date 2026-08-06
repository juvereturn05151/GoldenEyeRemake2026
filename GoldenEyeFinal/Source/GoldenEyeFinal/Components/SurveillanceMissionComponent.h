#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Mission/MissionRelevantActor.h"
#include "SurveillanceMissionComponent.generated.h"

UCLASS(ClassGroup = (Mission), meta = (BlueprintSpawnableComponent))
class GOLDENEYEFINAL_API USurveillanceMissionComponent : public UActorComponent, public IMissionRelevantActor
{
	GENERATED_BODY()

public:
	USurveillanceMissionComponent();

	UFUNCTION(BlueprintCallable, Category = "Mission|Surveillance")
	bool RegisterWithMissionSystem();

	UFUNCTION(BlueprintCallable, Category = "Mission|Surveillance")
	void UnregisterFromMissionSystem();

	UFUNCTION(BlueprintCallable, Category = "Mission|Surveillance")
	void NotifyCameraDestroyed(AActor* EventInstigator);

	UFUNCTION(BlueprintPure, Category = "Mission|Surveillance")
	bool IsAlreadyDestroyed() const;

	virtual FName GetMissionGroupId_Implementation() const override;
	virtual FName GetMissionActorId_Implementation() const override;
	virtual bool IsMissionActorCompleted_Implementation() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission|Surveillance")
	FName MissionGroupId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission|Surveillance")
	FName MissionActorId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission|Surveillance")
	bool bRegisterOnBeginPlay = true;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	bool bIsRegistered = false;
	bool bAlreadyDestroyed = false;
};
