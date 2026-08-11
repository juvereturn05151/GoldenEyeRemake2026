#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BorisCharacter.generated.h"

class AJamesBondCharacter;
class UAnimMontage;
class UNPCHealthComponent;

UENUM(BlueprintType)
enum class EBorisMissionState : uint8
{
	Idle,
	HandsUp,
	MovingToPointA,
	WaitingAtPointA,
	HurtReacting,
	MovingToComputer,
	ActivatingComputer,
	Completed,
	Dead
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBorisMissionEventSignature, class ABorisCharacter*, Boris);

UCLASS()
class GOLDENEYEFINAL_API ABorisCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABorisCharacter();

	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser
	) override;

	UFUNCTION(BlueprintCallable, Category = "Boris|Mission")
	void NotifyPlayerDetected(AJamesBondCharacter* DetectedBond);

	UFUNCTION(BlueprintCallable, Category = "Boris|Mission")
	void NotifyHandsUpFinished();

	UFUNCTION(BlueprintCallable, Category = "Boris|Mission")
	void NotifyHurtFinished();

	UFUNCTION(BlueprintCallable, Category = "Boris|Mission")
	void NotifyActivateComputerFinished();

	UFUNCTION(BlueprintPure, Category = "Boris|Mission")
	EBorisMissionState GetCurrentMissionState() const;

	UFUNCTION(BlueprintPure, Category = "Boris|Mission")
	bool IsMissionCompleted() const;

	UFUNCTION(BlueprintPure, Category = "Boris|Components")
	UNPCHealthComponent* GetHealthComponent() const;

	UPROPERTY(BlueprintAssignable, Category = "Boris|Mission")
	FBorisMissionEventSignature OnBorisSawPlayer;

	UPROPERTY(BlueprintAssignable, Category = "Boris|Mission")
	FBorisMissionEventSignature OnHandsUpStarted;

	UPROPERTY(BlueprintAssignable, Category = "Boris|Mission")
	FBorisMissionEventSignature OnReachedPointA;

	UPROPERTY(BlueprintAssignable, Category = "Boris|Mission")
	FBorisMissionEventSignature OnBorisProvoked;

	UPROPERTY(BlueprintAssignable, Category = "Boris|Mission")
	FBorisMissionEventSignature OnReachedComputer;

	UPROPERTY(BlueprintAssignable, Category = "Boris|Mission")
	FBorisMissionEventSignature OnComputerActivationStarted;

	UPROPERTY(BlueprintAssignable, Category = "Boris|Mission")
	FBorisMissionEventSignature OnBorisMissionCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Boris|Mission")
	FBorisMissionEventSignature OnBorisDiedBeforeMissionComplete;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boris|Components")
	TObjectPtr<UNPCHealthComponent> HealthComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boris|Mission")
	TObjectPtr<AActor> PointA = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boris|Mission")
	TObjectPtr<AActor> ComputerTarget = nullptr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Boris|Mission")
	EBorisMissionState CurrentMissionState = EBorisMissionState::Idle;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Boris|Mission")
	bool bMissionCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boris|Movement", meta = (ClampMin = "1.0"))
	float MoveAcceptanceRadius = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boris|Animation")
	TObjectPtr<UAnimMontage> HandsUpMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boris|Animation")
	TObjectPtr<UAnimMontage> HurtMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boris|Animation")
	TObjectPtr<UAnimMontage> ActivateComputerMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boris|Animation")
	TObjectPtr<UAnimMontage> DeathMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boris|Mission Event")
	bool bBroadcastMissionEventOnCompletion = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boris|Mission Event")
	FName MissionCompletedEventTag = TEXT("Boris.MissionCompleted");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boris|Mission Event")
	FName MissionCompletedContextId = TEXT("BorisComputer");

private:
	UFUNCTION()
	void HandleDamageTaken(float DamageAmount);

	UFUNCTION()
	void HandleDeath();

	void StartHandsUp();
	void MoveToPointA();
	void EnterWaitingAtPointA();
	void StartHurtReaction();
	void MoveToComputer();
	void StartComputerActivation();
	void CompleteBorisMission();
	void StopMissionMovement();
	void FaceActor(AActor* TargetActor);
	void SetMissionState(EBorisMissionState NewState);
	void BroadcastBorisMissionCompletedEvent();
	bool CanStartMissionProgression() const;
	bool IsDead() const;

	UFUNCTION()
	void CheckMoveArrival();

	void StartMoveArrivalCheck();
	void StopMoveArrivalCheck();
	AActor* GetCurrentMoveTarget() const;

	bool bDeathHandled = false;
	FTimerHandle MoveArrivalCheckTimer;
};
