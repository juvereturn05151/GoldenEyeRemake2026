/*
Name: Ju-ve Chankasemporn
E-mail: juvereturn@gmail.com
@2026 MyLoyalFans Productions
*/

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "SWATAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Hearing;
class UAISenseConfig_Sight;
class ASWATEnemyCharacter;
class UBehaviorTree;

UCLASS()
class GOLDENEYEFINAL_API ASWATAIController : public AAIController
{
	GENERATED_BODY()

public:
	ASWATAIController();

	UFUNCTION(BlueprintPure, Category = "SWAT|AI")
	AActor* GetTargetActor() const;

	UFUNCTION(BlueprintPure, Category = "SWAT|AI")
	FVector GetLastKnownLocation() const;

	UFUNCTION(BlueprintPure, Category = "SWAT|AI")
	FVector GetLastHeardLocation() const;

	UFUNCTION(BlueprintPure, Category = "SWAT|AI")
	bool HasLineOfSight() const;

	UFUNCTION(BlueprintPure, Category = "SWAT|AI")
	bool ShouldInvestigate() const;

	UFUNCTION(BlueprintCallable, Category = "SWAT|AI")
	void CompleteInvestigation();

	UFUNCTION(BlueprintCallable, Category = "SWAT|AI")
	void CompleteSearch();

	UFUNCTION(BlueprintCallable, Category = "SWAT|AI")
	void SetIsSearching(bool bNewSearching);

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SWAT|AI")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|AI|Perception", meta = (ClampMin = "0.0"))
	float SightRadius = 2000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|AI|Perception", meta = (ClampMin = "0.0"))
	float LoseSightRadius = 2500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|AI|Perception", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float PeripheralVisionHalfAngleDegrees = 75.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|AI|Perception", meta = (ClampMin = "0.0"))
	float HearingRange = 1600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|AI|Perception", meta = (ClampMin = "0.0"))
	float SightMaxAge = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|AI|Perception", meta = (ClampMin = "0.0"))
	float HearingMaxAge = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SWAT|Debug")
	bool bDebugPerception = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SWAT|Debug", meta = (ClampMin = "0.01"))
	float PerceptionDebugInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SWAT|Debug", meta = (ClampMin = "0.0"))
	float DebugDrawDuration = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SWAT|Debug", meta = (ClampMin = "0.0"))
	float DebugSphereRadius = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SWAT|Debug")
	float DebugTextHeight = 140.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SWAT|Debug", meta = (ClampMin = "0.0"))
	float DebugLineThickness = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SWAT|AI")
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SWAT|AI")
	FVector LastKnownLocation = FVector::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SWAT|AI")
	bool bHasValidLastKnownLocation = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SWAT|AI")
	FVector LastHeardLocation = FVector::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SWAT|AI")
	bool bHasValidLastHeardLocation = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SWAT|AI")
	bool bHasLineOfSight = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SWAT|AI")
	bool bShouldInvestigate = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SWAT|AI")
	FVector HomeLocation = FVector::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SWAT|AI")
	bool bIsSearching = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|AI|Combat", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float TooCloseDistance = 400.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|AI|Combat", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float PreferredMaximumDistance = 1400.0f;

private:
	UFUNCTION()
	void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void HandleControlledSWATStateChanged();

	void ConfigurePerception();
	void HandleSightStimulus(AActor* Actor, const FAIStimulus& Stimulus);
	void HandleHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus);
	void BindControlledSWAT(APawn* InPawn);
	void UnbindControlledSWAT();
	void SyncBlackboard();
	void SyncPerceptionBlackboard();
	void SyncSWATStateBlackboard();
	void SyncCombatRangeBlackboard();
	void ClearCombatRangeBlackboard(UBlackboardComponent* BlackboardComponent) const;
	bool IsPlayerPawn(AActor* Actor) const;
	void StartPerceptionDebug();
	void StopPerceptionDebug();
	void DrawPerceptionDebug();
	void DrawSightDebug() const;
	void DrawLastKnownLocationDebug() const;
	void DrawLastHeardLocationDebug() const;
	void DrawPerceptionStateText() const;

	FTimerHandle PerceptionDebugTimerHandle;
	TObjectPtr<ASWATEnemyCharacter> ControlledSWAT;
};
