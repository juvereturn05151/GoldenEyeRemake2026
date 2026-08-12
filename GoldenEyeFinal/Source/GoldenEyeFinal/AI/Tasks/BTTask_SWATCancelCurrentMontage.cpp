#include "BTTask_SWATCancelCurrentMontage.h"

#include "../../Characters/SWATEnemyCharacter.h"
#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"

UBTTask_SWATCancelCurrentMontage::UBTTask_SWATCancelCurrentMontage()
{
	NodeName = TEXT("SWAT Cancel Current Montage");
}

EBTNodeResult::Type UBTTask_SWATCancelCurrentMontage::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory
)
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	ASWATEnemyCharacter* SWATCharacter = Cast<ASWATEnemyCharacter>(AIController->GetPawn());
	if (!SWATCharacter)
	{
		return EBTNodeResult::Failed;
	}

	USkeletalMeshComponent* MeshComponent = SWATCharacter->GetMesh();
	if (!MeshComponent)
	{
		return EBTNodeResult::Failed;
	}

	UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
	if (!AnimInstance)
	{
		return EBTNodeResult::Failed;
	}

	const bool bWasMontagePlaying = AnimInstance->IsAnyMontagePlaying();

	if (bWasMontagePlaying)
	{
		AnimInstance->Montage_Stop(MontageBlendOutTime);
	}

	if (bLogResult)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[SWAT] %s cancel montage task %s"),
			*SWATCharacter->GetName(),
			bWasMontagePlaying ? TEXT("stopped an active montage") : TEXT("found no active montage")
		);
	}

	return bWasMontagePlaying || !bFailIfNoMontageWasPlaying
		? EBTNodeResult::Succeeded
		: EBTNodeResult::Failed;
}
