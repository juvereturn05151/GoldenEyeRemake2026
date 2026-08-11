#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BorisComputerActor.generated.h"

class USceneComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBorisComputerActivatedSignature, class ABorisComputerActor*, Computer);

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

	UPROPERTY(BlueprintAssignable, Category = "Boris|Computer")
	FBorisComputerActivatedSignature OnComputerActivated;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boris|Computer")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boris|Computer")
	bool bActivated = false;
};
