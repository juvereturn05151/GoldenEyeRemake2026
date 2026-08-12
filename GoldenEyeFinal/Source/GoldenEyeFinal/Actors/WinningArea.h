#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WinningArea.generated.h"

class UBoxComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWinningAreaTriggeredSignature, class AJamesBondCharacter*, Bond);

UCLASS()
class GOLDENEYEFINAL_API AWinningArea : public AActor
{
	GENERATED_BODY()

public:
	AWinningArea();

	UPROPERTY(BlueprintAssignable, Category = "Winning Area")
	FWinningAreaTriggeredSignature OnWinningAreaTriggered;

	UFUNCTION(BlueprintCallable, Category = "Winning Area")
	void ResetWinningArea();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Winning Area|Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Winning Area")
	bool bTriggerOnlyOnce = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Winning Area")
	bool bPauseGameOnWin = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Winning Area")
	FVector TriggerBoxExtent = FVector(180.0f, 180.0f, 120.0f);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Winning Area")
	bool bHasTriggered = false;

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
};
