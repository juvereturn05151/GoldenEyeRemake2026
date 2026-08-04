#include "BTTask_SWATFireBurst.h"

#include "../../Characters/SWATEnemyCharacter.h"
#include "../../Components/SWATCombatComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"

UBTTask_SWATFireBurst::UBTTask_SWATFireBurst()
{
	NodeName = TEXT("SWAT Fire Burst");
	bCreateNodeInstance = true;
	bNotifyTaskFinished = true;

	TargetActorKey.SelectedKeyName = TEXT("TargetActor");
	TargetActorKey.AddObjectFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UBTTask_SWATFireBurst, TargetActorKey),
		AActor::StaticClass()
	);
}

EBTNodeResult::Type UBTTask_SWATFireBurst::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();

	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	ASWATEnemyCharacter* SWATCharacter = Cast<ASWATEnemyCharacter>(AIController->GetPawn());

	if (!SWATCharacter)
	{
		return EBTNodeResult::Failed;
	}

	USWATCombatComponent* CombatComponent = SWATCharacter->GetCombatComponent();

	if (!CombatComponent)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();

	if (!BlackboardComponent)
	{
		return EBTNodeResult::Failed;
	}

	AActor* TargetActor = Cast<AActor>(
		BlackboardComponent->GetValueAsObject(TargetActorKey.SelectedKeyName)
	);

	if (!TargetActor)
	{
		return EBTNodeResult::Failed;
	}

	CachedOwnerComp = &OwnerComp;
	CachedCombatComponent = CombatComponent;

	CombatComponent->OnBurstFinished.RemoveDynamic(
		this,
		&UBTTask_SWATFireBurst::HandleBurstFinished
	);
	CombatComponent->OnBurstFinished.AddDynamic(
		this,
		&UBTTask_SWATFireBurst::HandleBurstFinished
	);

	if (!CombatComponent->StartCombatBurst(TargetActor))
	{
		UnbindBurstDelegate();
		return EBTNodeResult::Failed;
	}

	return CombatComponent->IsBurstActive()
		? EBTNodeResult::InProgress
		: EBTNodeResult::Succeeded;
}

EBTNodeResult::Type UBTTask_SWATFireBurst::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (USWATCombatComponent* CombatComponent = CachedCombatComponent.Get())
	{
		UnbindBurstDelegate();
		CombatComponent->StopCombatBurst();
	}

	CachedOwnerComp.Reset();
	CachedCombatComponent.Reset();

	return EBTNodeResult::Aborted;
}

void UBTTask_SWATFireBurst::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	UnbindBurstDelegate();
	CachedOwnerComp.Reset();
	CachedCombatComponent.Reset();

	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBTTask_SWATFireBurst::HandleBurstFinished()
{
	UBehaviorTreeComponent* OwnerComp = CachedOwnerComp.Get();

	if (!OwnerComp)
	{
		UnbindBurstDelegate();
		CachedCombatComponent.Reset();
		return;
	}

	UnbindBurstDelegate();
	CachedCombatComponent.Reset();

	FinishLatentTask(*OwnerComp, EBTNodeResult::Succeeded);
}

void UBTTask_SWATFireBurst::UnbindBurstDelegate()
{
	if (USWATCombatComponent* CombatComponent = CachedCombatComponent.Get())
	{
		CombatComponent->OnBurstFinished.RemoveDynamic(
			this,
			&UBTTask_SWATFireBurst::HandleBurstFinished
		);
	}
}
