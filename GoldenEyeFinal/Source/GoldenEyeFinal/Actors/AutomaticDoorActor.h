/*
Name: Ju-ve Chankasemporn
E-mail: juvereturn@gmail.com
@2026 MyLoyalFans Productions
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AutomaticDoorActor.generated.h"

class UBoxComponent;
class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class GOLDENEYEFINAL_API AAutomaticDoorActor : public AActor
{
	GENERATED_BODY()

public:
	AAutomaticDoorActor();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Door")
	void OpenDoor();

	UFUNCTION(BlueprintCallable, Category = "Door")
	void CloseDoor();

	UFUNCTION(BlueprintPure, Category = "Door")
	bool IsOpen() const;

	UFUNCTION(BlueprintCallable, Category = "Door|Lock")
	void SetLocked(bool bNewLocked);

	UFUNCTION(BlueprintCallable, Category = "Door|Lock")
	void LockDoor();

	UFUNCTION(BlueprintCallable, Category = "Door|Lock")
	void UnlockDoor();

	UFUNCTION(BlueprintPure, Category = "Door|Lock")
	bool IsLocked() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door|Components")
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door|Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door|Movement")
	FVector OpenOffset = FVector(0.0f, 180.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door|Movement", meta = (ClampMin = "0.01"))
	float OpenDuration = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door|Movement", meta = (ClampMin = "0.01"))
	float CloseDuration = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door|Movement", meta = (ClampMin = "0.0"))
	float CloseDelay = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door|Movement")
	bool bStartOpen = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door|Lock")
	bool bLocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door|Detection")
	FVector TriggerBoxExtent = FVector(200.0f, 200.0f, 120.0f);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Door|State")
	bool bIsOpen = false;

	UFUNCTION(BlueprintImplementableEvent, Category = "Door|Events")
	void OnDoorOpenStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Door|Events")
	void OnDoorCloseStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Door|Events")
	void OnDoorLocked();

	UFUNCTION(BlueprintImplementableEvent, Category = "Door|Events")
	void OnDoorUnlocked();

private:
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

	void UpdateDoorMovement(float DeltaSeconds);
	void RefreshDoorTarget();
	void RemoveInvalidOverlappingUsers();
	void RefreshOverlappingUsersFromTrigger();
	bool CanUseDoor(AActor* Actor) const;

	FVector ClosedRelativeLocation = FVector::ZeroVector;
	FVector OpenRelativeLocation = FVector::ZeroVector;
	float CurrentOpenAlpha = 0.0f;
	float TargetOpenAlpha = 0.0f;
	float TimeSinceLastUserLeft = 0.0f;
	TSet<TWeakObjectPtr<AActor>> OverlappingUsers;
};
