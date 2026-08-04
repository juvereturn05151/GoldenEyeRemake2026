#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_SWATUpdateCombatState.generated.h"

UCLASS()
class GOLDENEYEFINAL_API UBTService_SWATUpdateCombatState : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_SWATUpdateCombatState();

protected:
	virtual void OnBecomeRelevant(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory
	) override;

	virtual void TickNode(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		float DeltaSeconds
	) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SWAT|Combat", meta = (ClampMin = "0.0"))
	float TooCloseDistance = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SWAT|Combat", meta = (ClampMin = "0.0"))
	float PreferredMaximumDistance = 1400.0f;

private:
	static const FName TargetActorKeyName;
	static const FName DistanceToTargetKeyName;
	static const FName IsTooFarKeyName;
	static const FName IsTooCloseKeyName;
	static const FName IsInPreferredRangeKeyName;

	void UpdateCombatState(UBehaviorTreeComponent& OwnerComp);
	void ClearRangeBlackboard(UBlackboardComponent* BlackboardComponent) const;
};
