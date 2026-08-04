#include "BTService_SWATUpdateCombatState.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"

const FName UBTService_SWATUpdateCombatState::TargetActorKeyName(TEXT("TargetActor"));
const FName UBTService_SWATUpdateCombatState::DistanceToTargetKeyName(TEXT("DistanceToTarget"));
const FName UBTService_SWATUpdateCombatState::IsTooFarKeyName(TEXT("IsTooFar"));
const FName UBTService_SWATUpdateCombatState::IsTooCloseKeyName(TEXT("IsTooClose"));
const FName UBTService_SWATUpdateCombatState::IsInPreferredRangeKeyName(TEXT("IsInPreferredRange"));

UBTService_SWATUpdateCombatState::UBTService_SWATUpdateCombatState()
{
	NodeName = TEXT("SWAT Update Combat State");
	Interval = 0.2f;
	RandomDeviation = 0.05f;
	bNotifyBecomeRelevant = true;
}

void UBTService_SWATUpdateCombatState::OnBecomeRelevant(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory
)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	UpdateCombatState(OwnerComp);
}

void UBTService_SWATUpdateCombatState::TickNode(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	float DeltaSeconds
)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UpdateCombatState(OwnerComp);
}

void UBTService_SWATUpdateCombatState::UpdateCombatState(
	UBehaviorTreeComponent& OwnerComp
)
{
	UBlackboardComponent* BlackboardComponent =
		OwnerComp.GetBlackboardComponent();

	if (!BlackboardComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat Range Failed] Blackboard invalid"));
		return;
	}

	const AAIController* AIController = OwnerComp.GetAIOwner();

	if (!AIController)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat Range Failed] AIController invalid"));
		ClearRangeBlackboard(BlackboardComponent);
		return;
	}

	const APawn* ControlledPawn = AIController->GetPawn();

	if (!ControlledPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat Range Failed] Pawn invalid"));
		ClearRangeBlackboard(BlackboardComponent);
		return;
	}

	const AActor* TargetActor = Cast<AActor>(
		BlackboardComponent->GetValueAsObject(TargetActorKeyName)
	);

	if (!TargetActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SWAT Combat Range Failed] TargetActor invalid"));
		ClearRangeBlackboard(BlackboardComponent);
		return;
	}

	const float DistanceToTarget = FVector::Dist(
		ControlledPawn->GetActorLocation(),
		TargetActor->GetActorLocation()
	);

	BlackboardComponent->SetValueAsFloat(
		DistanceToTargetKeyName,
		DistanceToTarget
	);

	bool bTooClose = false;
	bool bPreferred = false;
	bool bTooFar = false;

	if (DistanceToTarget < TooCloseDistance)
	{
		bTooClose = true;
	}
	else if (DistanceToTarget <= PreferredMaximumDistance)
	{
		bPreferred = true;
	}
	else
	{
		bTooFar = true;
	}

	BlackboardComponent->SetValueAsBool(IsTooCloseKeyName, bTooClose);
	BlackboardComponent->SetValueAsBool(IsInPreferredRangeKeyName, bPreferred);
	BlackboardComponent->SetValueAsBool(IsTooFarKeyName, bTooFar);

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[SWAT Combat Range] Pawn=%s Target=%s Distance=%.2f TooClose=%s Preferred=%s TooFar=%s"),
		*GetNameSafe(ControlledPawn),
		*GetNameSafe(TargetActor),
		DistanceToTarget,
		bTooClose ? TEXT("true") : TEXT("false"),
		bPreferred ? TEXT("true") : TEXT("false"),
		bTooFar ? TEXT("true") : TEXT("false")
	);
}

void UBTService_SWATUpdateCombatState::ClearRangeBlackboard(
	UBlackboardComponent* BlackboardComponent
) const
{
	if (!BlackboardComponent)
	{
		return;
	}

	BlackboardComponent->SetValueAsFloat(DistanceToTargetKeyName, 0.0f);
	BlackboardComponent->SetValueAsBool(IsTooCloseKeyName, false);
	BlackboardComponent->SetValueAsBool(IsInPreferredRangeKeyName, false);
	BlackboardComponent->SetValueAsBool(IsTooFarKeyName, false);
}
