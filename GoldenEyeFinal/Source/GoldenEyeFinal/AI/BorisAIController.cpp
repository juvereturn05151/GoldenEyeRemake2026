#include "BorisAIController.h"

#include "../Characters/BorisCharacter.h"
#include "../Characters/JamesBondCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ABorisAIController::ABorisAIController()
{
	PrimaryActorTick.bCanEverTick = false;

	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	SetPerceptionComponent(*AIPerceptionComponent);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	ConfigurePerception();
}

void ABorisAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ControlledBoris = Cast<ABorisCharacter>(InPawn);
	ConfigurePerception();
	StartSightPolling();
	CheckBondSight();
}

void ABorisAIController::OnUnPossess()
{
	StopSightPolling();
	ControlledBoris = nullptr;

	Super::OnUnPossess();
}

void ABorisAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopSightPolling();

	Super::EndPlay(EndPlayReason);
}

void ABorisAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	if (ControlledBoris)
	{
		ControlledBoris->NotifyMissionMoveCompleted(Result.IsSuccess());
	}
}

void ABorisAIController::HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!ControlledBoris || !IsBond(Actor))
	{
		return;
	}

	if (Stimulus.Type != UAISense::GetSenseID<UAISense_Sight>() || !Stimulus.WasSuccessfullySensed())
	{
		return;
	}

	if (bDebugPerception)
	{
		UE_LOG(LogTemp, Log, TEXT("BORIS: Sight perception detected Bond"));
	}

	ControlledBoris->NotifyPlayerDetected(Cast<AJamesBondCharacter>(Actor));
}

void ABorisAIController::ConfigurePerception()
{
	if (!AIPerceptionComponent || !SightConfig)
	{
		return;
	}

	SightConfig->SightRadius = SightRadius;
	SightConfig->LoseSightRadius = LoseSightRadius;
	SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionHalfAngleDegrees;
	SightConfig->SetMaxAge(SightMaxAge);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	AIPerceptionComponent->ConfigureSense(*SightConfig);
	AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
	AIPerceptionComponent->OnTargetPerceptionUpdated.RemoveDynamic(
		this,
		&ABorisAIController::HandleTargetPerceptionUpdated
	);
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
		this,
		&ABorisAIController::HandleTargetPerceptionUpdated
	);

	AIPerceptionComponent->RequestStimuliListenerUpdate();
}

void ABorisAIController::StartSightPolling()
{
	if (!bUseSightPollingBackup)
	{
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(SightPollingTimerHandle);
	World->GetTimerManager().SetTimer(
		SightPollingTimerHandle,
		this,
		&ABorisAIController::CheckBondSight,
		SightPollingInterval,
		true
	);
}

void ABorisAIController::StopSightPolling()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SightPollingTimerHandle);
	}
}

void ABorisAIController::CheckBondSight()
{
	if (!ControlledBoris)
	{
		return;
	}

	AJamesBondCharacter* Bond = Cast<AJamesBondCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));

	if (!Bond || !HasSightToBond(Bond))
	{
		return;
	}

	if (bDebugPerception)
	{
		UE_LOG(LogTemp, Log, TEXT("BORIS: Sight polling detected Bond"));
	}

	ControlledBoris->NotifyPlayerDetected(Bond);
}

bool ABorisAIController::HasSightToBond(AJamesBondCharacter* Bond) const
{
	const APawn* ControlledPawn = GetPawn();

	if (!ControlledPawn || !Bond)
	{
		return false;
	}

	FVector EyeLocation;
	FRotator EyeRotation;
	ControlledPawn->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	const FVector BondLocation = Bond->GetActorLocation();
	const FVector ToBond = BondLocation - EyeLocation;

	if (ToBond.SizeSquared() > FMath::Square(SightRadius))
	{
		return false;
	}

	const FVector Forward = EyeRotation.Vector().GetSafeNormal2D();
	const FVector DirectionToBond = ToBond.GetSafeNormal2D();

	if (!Forward.IsNearlyZero() && !DirectionToBond.IsNearlyZero())
	{
		const float Dot = FVector::DotProduct(Forward, DirectionToBond);
		const float MinDot = FMath::Cos(FMath::DegreesToRadians(PeripheralVisionHalfAngleDegrees));

		if (Dot < MinDot)
		{
			return false;
		}
	}

	return LineOfSightTo(Bond);
}

bool ABorisAIController::IsBond(AActor* Actor) const
{
	return Cast<AJamesBondCharacter>(Actor) != nullptr;
}
