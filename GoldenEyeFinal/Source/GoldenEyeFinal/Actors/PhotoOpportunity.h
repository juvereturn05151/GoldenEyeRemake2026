/*
Name: Ju-ve Chankasemporn
E-mail: juvereturn@gmail.com
@2026 MyLoyalFans Productions
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PhotoOpportunity.generated.h"

class AJamesBondCharacter;
class UBoxComponent;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPhotoOpportunityEvent);

UCLASS()
class GOLDENEYEFINAL_API APhotoOpportunity : public AActor
{
	GENERATED_BODY()

public:
	APhotoOpportunity();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Photo Opportunity")
	void TryTakePhoto();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Photo Opportunity")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Photo Opportunity")
	TObjectPtr<AActor> PhotoTarget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Photo Opportunity", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float FacingDotThreshold = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Photo Opportunity|Mission")
	bool bBroadcastMissionEventOnPhotoTaken = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Photo Opportunity|Mission", meta = (EditCondition = "bBroadcastMissionEventOnPhotoTaken"))
	FName MissionEventTag = TEXT("Photo.Taken");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Photo Opportunity|Mission", meta = (EditCondition = "bBroadcastMissionEventOnPhotoTaken"))
	FName MissionContextId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Photo Opportunity|Audio")
	TObjectPtr<USoundBase> PhotoTakenSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Photo Opportunity|Audio", meta = (ClampMin = "0.0"))
	float PhotoTakenSoundVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Photo Opportunity|Audio", meta = (ClampMin = "0.01"))
	float PhotoTakenSoundPitch = 1.0f;

	UPROPERTY(BlueprintAssignable, Category = "Photo Opportunity")
	FPhotoOpportunityEvent OnPhotoAvailable;

	UPROPERTY(BlueprintAssignable, Category = "Photo Opportunity")
	FPhotoOpportunityEvent OnPhotoUnavailable;

	UPROPERTY(BlueprintAssignable, Category = "Photo Opportunity")
	FPhotoOpportunityEvent OnPhotoTaken;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<AJamesBondCharacter> OverlappingBond;

	bool bPlayerInside = false;
	bool bPhotoAvailable = false;
	bool bPhotoTaken = false;

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

	void UpdatePhotoAvailability();
	bool IsPhotoValid(bool bLogFailure = false) const;
	bool GetCameraView(FVector& OutCameraLocation, FVector& OutCameraForward) const;
	FVector GetPhotoTargetViewLocation() const;
	bool IsFacingTarget(const FVector& CameraLocation, const FVector& CameraForward, bool bLogFailure = false) const;
	bool IsTargetVisible(const FVector& CameraLocation, bool bLogFailure = false) const;
	bool IsTraceHitOnPhotoTarget(const FHitResult& Hit) const;
	void SetPhotoAvailable(bool bNewAvailable);
	void PlayPhotoTakenSound() const;
	void BroadcastPhotoMissionEvent();
};
