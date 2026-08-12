/*
Name: Ju-ve Chankasemporn
E-mail: juvereturn@gmail.com
@2026 MyLoyalFans Productions
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CopyOpportunity.generated.h"

class AJamesBondCharacter;
class UAudioComponent;
class UBoxComponent;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCopyOpportunityEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCopyProgressChangedEvent, float, Progress);

UCLASS()
class GOLDENEYEFINAL_API ACopyOpportunity : public AActor
{
	GENERATED_BODY()

public:
	ACopyOpportunity();

	UFUNCTION(BlueprintCallable, Category = "Copy Opportunity")
	void TryStartCopy();

	UFUNCTION(BlueprintPure, Category = "Copy Opportunity")
	float GetCurrentCopyProgress() const;

	UFUNCTION(BlueprintPure, Category = "Copy Opportunity")
	bool IsPlayerInside() const;

	UFUNCTION(BlueprintPure, Category = "Copy Opportunity")
	bool IsCopyInProgress() const;

	UFUNCTION(BlueprintPure, Category = "Copy Opportunity")
	bool IsCopyCompleted() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Copy Opportunity")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Copy Opportunity|Sound")
	TObjectPtr<UAudioComponent> CopyingAudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Copy Opportunity", meta = (ClampMin = "0.01"))
	float CopyDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Copy Opportunity|Sound")
	TObjectPtr<USoundBase> CopyDataSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Copy Opportunity|Sound")
	TObjectPtr<USoundBase> CopyingSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Copy Opportunity|Sound")
	TObjectPtr<USoundBase> CopyCompletedSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Copy Opportunity")
	float CurrentCopyProgress = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Copy Opportunity")
	bool bPlayerInside = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Copy Opportunity")
	bool bCopyInProgress = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Copy Opportunity")
	bool bCopyCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Copy Opportunity|Mission")
	bool bBroadcastMissionEventOnCopyCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Copy Opportunity|Mission", meta = (EditCondition = "bBroadcastMissionEventOnCopyCompleted"))
	FName MissionEventTag = TEXT("Copy.Completed");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Copy Opportunity|Mission", meta = (EditCondition = "bBroadcastMissionEventOnCopyCompleted"))
	FName MissionContextId = NAME_None;

	UPROPERTY(BlueprintAssignable, Category = "Copy Opportunity")
	FCopyOpportunityEvent OnCopyAvailable;

	UPROPERTY(BlueprintAssignable, Category = "Copy Opportunity")
	FCopyOpportunityEvent OnCopyUnavailable;

	UPROPERTY(BlueprintAssignable, Category = "Copy Opportunity")
	FCopyOpportunityEvent OnCopyStarted;

	UPROPERTY(BlueprintAssignable, Category = "Copy Opportunity")
	FCopyProgressChangedEvent OnCopyProgressChanged;

	UPROPERTY(BlueprintAssignable, Category = "Copy Opportunity")
	FCopyOpportunityEvent OnCopyCancelled;

	UPROPERTY(BlueprintAssignable, Category = "Copy Opportunity")
	FCopyOpportunityEvent OnCopyCompleted;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY()
	TObjectPtr<AJamesBondCharacter> OverlappingBond;

	FTimerHandle CopyProgressTimer;
	float CopyElapsedTime = 0.0f;
	float NextProgressLogMilestone = 0.25f;

	UFUNCTION()
	void HandleTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void HandleTriggerEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	void UpdateCopyProgress();
	void CompleteCopy();
	void CancelCopy();
	void ClearCopyTimer();
	void BroadcastCopyMissionEvent();
	void PlayCopySound(USoundBase* Sound) const;
	void StartCopyingSound();
	void StopCopyingSound();
};
