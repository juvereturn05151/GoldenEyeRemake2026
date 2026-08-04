#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTTask_SWATFireBurst.generated.h"

class ASWATEnemyCharacter;
class UBehaviorTreeComponent;
class USWATCombatComponent;

UCLASS()
class GOLDENEYEFINAL_API UBTTask_SWATFireBurst : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SWATFireBurst();

	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory
	) override;

	virtual EBTNodeResult::Type AbortTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory
	) override;

	virtual void OnTaskFinished(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		EBTNodeResult::Type TaskResult
	) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

private:
	UFUNCTION()
	void HandleBurstFinished();

	void UnbindBurstDelegate();

	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
	TWeakObjectPtr<USWATCombatComponent> CachedCombatComponent;
};
