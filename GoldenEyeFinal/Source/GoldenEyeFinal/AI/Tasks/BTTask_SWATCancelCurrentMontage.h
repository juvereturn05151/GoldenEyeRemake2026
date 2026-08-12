#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SWATCancelCurrentMontage.generated.h"

UCLASS()
class GOLDENEYEFINAL_API UBTTask_SWATCancelCurrentMontage : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SWATCancelCurrentMontage();

	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory
	) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SWAT|Animation", meta = (ClampMin = "0.0"))
	float MontageBlendOutTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SWAT|Animation")
	bool bFailIfNoMontageWasPlaying = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SWAT|Debug")
	bool bLogResult = false;
};
