#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BorisComputerActor.generated.h"

class AJamesBondCharacter;
class UBoxComponent;
class USceneComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBorisComputerActivatedSignature, class ABorisComputerActor*, Computer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBorisComputerInteractionSignature, class ABorisComputerActor*, Computer);

UCLASS()
class GOLDENEYEFINAL_API ABorisComputerActor : public AActor
{
	GENERATED_BODY()

public:
	ABorisComputerActor();

	UFUNCTION(BlueprintCallable, Category = "Boris|Computer")
	bool ActivateComputer();

	UFUNCTION(BlueprintPure, Category = "Boris|Computer")
	bool IsActivated() const;

	UFUNCTION(BlueprintCallable, Category = "Boris|Computer")
	bool TryInteract(AJamesBondCharacter* Bond);

	UFUNCTION(BlueprintPure, Category = "Boris|Computer")
	bool IsInteractionAvailable() const;

	UFUNCTION(BlueprintPure, Category = "Boris|Computer")
	bool HasBondInsideTrigger() const;

	UPROPERTY(BlueprintAssignable, Category = "Boris|Computer")
	FBorisComputerActivatedSignature OnComputerActivated;

	UPROPERTY(BlueprintAssignable, Category = "Boris|Computer")
	FBorisComputerInteractionSignature OnComputerInteractionAvailable;

	UPROPERTY(BlueprintAssignable, Category = "Boris|Computer")
	FBorisComputerInteractionSignature OnComputerInteractionUnavailable;

	UPROPERTY(BlueprintAssignable, Category = "Boris|Computer")
	FBorisComputerInteractionSignature OnComputerInteracted;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boris|Computer")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boris|Computer")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boris|Computer")
	bool bActivated = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boris|Computer|Interaction")
	bool bAllowRepeatedInteraction = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boris|Computer|Interaction")
	FText InteractionPromptText = FText::FromString(TEXT("Use Computer"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boris|Mission Event")
	bool bBroadcastMissionEventOnActivated = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boris|Mission Event")
	FName ActivationMissionEventTag = TEXT("Computer.Activated");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boris|Mission Event")
	FName ActivationMissionContextId = TEXT("MainComputer");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boris|Mission Event")
	bool bBroadcastMissionEventOnInteraction = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boris|Mission Event")
	FName InteractionMissionEventTag = TEXT("Computer.DataDownloaded");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boris|Mission Event")
	FName InteractionMissionContextId = TEXT("MainComputer");

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Boris|Computer|Interaction")
	bool bBondInsideTrigger = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Boris|Computer|Interaction")
	bool bInteractionCompleted = false;

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

	void SetBondInteractionAvailable(AJamesBondCharacter* Bond, bool bAvailable);
	void BroadcastActivationMissionEvent();
	void BroadcastInteractionMissionEvent(AJamesBondCharacter* Bond);

	UPROPERTY()
	TObjectPtr<AJamesBondCharacter> OverlappingBond;
};
