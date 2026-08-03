#include "SWATAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"

ASWATAIController::ASWATAIController()
{
	PrimaryActorTick.bCanEverTick = false;

	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	SetPerceptionComponent(*AIPerceptionComponent);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));

	ConfigurePerception();
}

void ASWATAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ConfigurePerception();

	if (BehaviorTreeAsset)
	{
		RunBehaviorTree(BehaviorTreeAsset);
	}
}

AActor* ASWATAIController::GetTargetActor() const
{
	return TargetActor;
}

FVector ASWATAIController::GetLastKnownLocation() const
{
	return LastKnownLocation;
}

FVector ASWATAIController::GetLastHeardLocation() const
{
	return LastHeardLocation;
}

bool ASWATAIController::HasLineOfSight() const
{
	return bHasLineOfSight;
}

bool ASWATAIController::ShouldInvestigate() const
{
	return bShouldInvestigate;
}

void ASWATAIController::HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		HandleSightStimulus(Actor, Stimulus);
		return;
	}

	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		HandleHearingStimulus(Stimulus);
	}
}

void ASWATAIController::ConfigurePerception()
{
	if (!AIPerceptionComponent)
	{
		return;
	}

	if (SightConfig)
	{
		SightConfig->SightRadius = SightRadius;
		SightConfig->LoseSightRadius = LoseSightRadius;
		SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionHalfAngleDegrees;
		SightConfig->SetMaxAge(SightMaxAge);
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

		AIPerceptionComponent->ConfigureSense(*SightConfig);
		AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
	}

	if (HearingConfig)
	{
		HearingConfig->HearingRange = HearingRange;
		HearingConfig->SetMaxAge(HearingMaxAge);
		HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
		HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
		HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;

		AIPerceptionComponent->ConfigureSense(*HearingConfig);
	}

	AIPerceptionComponent->OnTargetPerceptionUpdated.RemoveDynamic(
		this,
		&ASWATAIController::HandleTargetPerceptionUpdated
	);

	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
		this,
		&ASWATAIController::HandleTargetPerceptionUpdated
	);
}

void ASWATAIController::HandleSightStimulus(AActor* Actor, const FAIStimulus& Stimulus)
{
	if (!IsPlayerPawn(Actor))
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		TargetActor = Actor;
		bHasLineOfSight = true;
		bShouldInvestigate = false;
		LastKnownLocation = Actor->GetActorLocation();
		return;
	}

	bHasLineOfSight = false;
}

void ASWATAIController::HandleHearingStimulus(const FAIStimulus& Stimulus)
{
	if (!Stimulus.WasSuccessfullySensed())
	{
		return;
	}

	LastHeardLocation = Stimulus.StimulusLocation;
	bShouldInvestigate = true;
}

bool ASWATAIController::IsPlayerPawn(AActor* Actor) const
{
	const UWorld* World = GetWorld();

	if (!World || !Actor)
	{
		return false;
	}

	return Actor == UGameplayStatics::GetPlayerPawn(World, 0);
}
