#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "EnemySpawnerComponent.generated.h"

class AJamesBondCharacter;
class ASWATEnemyCharacter;
class ATargetPoint;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FEnemySpawnerBondSignature,
	AActor*,
	BondActor
);

UCLASS(ClassGroup = (Spawning), meta = (BlueprintSpawnableComponent))
class GOLDENEYEFINAL_API UEnemySpawnerComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	UEnemySpawnerComponent();

	UFUNCTION(BlueprintCallable, Category = "Enemy Spawner")
	void ResetSpawner();

	UFUNCTION(BlueprintPure, Category = "Enemy Spawner")
	int32 GetActivationCount() const;

	UFUNCTION(BlueprintPure, Category = "Enemy Spawner")
	float GetBondInsideTime() const;

	UPROPERTY(BlueprintAssignable, Category = "Enemy Spawner")
	FEnemySpawnerBondSignature OnBondEnteredSpawner;

	UPROPERTY(BlueprintAssignable, Category = "Enemy Spawner")
	FEnemySpawnerBondSignature OnSpawnerActivated;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Spawner")
	TSubclassOf<ASWATEnemyCharacter> SWATClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Spawner")
	TArray<TObjectPtr<ATargetPoint>> SpawnPoints;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Spawner", meta = (MakeEditWidget = "true"))
	TArray<FTransform> ManualSpawnTransforms;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Spawner", meta = (ClampMin = "0.0"))
	float RequiredBondInsideSeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Spawner", meta = (ClampMin = "1"))
	int32 SpawnCountPerActivation = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Spawner", meta = (ClampMin = "1"))
	int32 MaxActivationCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Spawner", meta = (ClampMin = "0.0"))
	float SpawnHorizontalJitterRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Spawner")
	float SpawnZOffset = 88.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Spawner")
	bool bResetInsideTimeOnBondExit = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Spawner|Alert")
	TArray<TObjectPtr<AActor>> AlertCameraActors;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Spawner|Alert", meta = (ClampMin = "0.0"))
	float AlertFlashDuration = 3.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy Spawner")
	int32 ActivationCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy Spawner")
	float BondInsideTime = 0.0f;

private:
	UFUNCTION()
	void HandleBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void HandleEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	bool IsBondActor(const AActor* Actor) const;
	bool CanActivate() const;
	void TriggerAlertCameras(AActor* BondActor);
	void ActivateSpawner();
	void SpawnSWAT(AActor* BondActor);
	FTransform GetSpawnTransform(int32 SpawnIndex) const;
	void AssignTargetToSpawnedSWAT(ASWATEnemyCharacter* SpawnedSWAT, AActor* BondActor) const;

	UPROPERTY()
	TObjectPtr<AActor> CurrentBondActor;

	int32 NextSpawnPointIndex = 0;
};
