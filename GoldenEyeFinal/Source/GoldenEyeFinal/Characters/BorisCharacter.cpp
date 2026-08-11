#include "BorisCharacter.h"

#include "../Actors/BorisComputerActor.h"
#include "../Components/NPCHealthComponent.h"
#include "../Mission/GameplayMissionSubsystem.h"
#include "../Mission/MissionTypes.h"
#include "AIController.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

ABorisCharacter::ABorisCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	bUseControllerRotationYaw = true;

	HealthComponent = CreateDefaultSubobject<UNPCHealthComponent>(TEXT("HealthComponent"));

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->bOrientRotationToMovement = false;
		MovementComponent->bUseControllerDesiredRotation = true;
		MovementComponent->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
	}
}

void ABorisCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComponent)
	{
		HealthComponent->OnDamageTaken.AddUniqueDynamic(this, &ABorisCharacter::HandleDamageTaken);
		HealthComponent->OnDeath.AddUniqueDynamic(this, &ABorisCharacter::HandleDeath);
	}
}

void ABorisCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopMoveArrivalCheck();

	Super::EndPlay(EndPlayReason);
}

float ABorisCharacter::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser
)
{
	if (IsDead() || !HealthComponent || DamageAmount <= 0.0f)
	{
		return 0.0f;
	}

	HealthComponent->ApplyDamage(DamageAmount);
	return DamageAmount;
}

void ABorisCharacter::NotifyPlayerDetected(AJamesBondCharacter* DetectedBond)
{
	if (!DetectedBond || !CanStartMissionProgression())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("BORIS: Player detected"));
	OnBorisSawPlayer.Broadcast(this);
	StartHandsUp();
}

void ABorisCharacter::NotifyHandsUpFinished()
{
	if (CurrentMissionState != EBorisMissionState::HandsUp || IsDead())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("BORIS: Hands up finished"));
	MoveToPointA();
}

void ABorisCharacter::NotifyHurtFinished()
{
	if (CurrentMissionState != EBorisMissionState::HurtReacting || IsDead())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("BORIS: Hurt reaction finished"));
	MoveToComputer();
}

void ABorisCharacter::NotifyActivateComputerFinished()
{
	if (CurrentMissionState != EBorisMissionState::ActivatingComputer || IsDead())
	{
		return;
	}

	AActor* TargetActor = ComputerTarget.Get();

	if (!TargetActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("BORIS: Cannot activate computer because ComputerTarget is not assigned"));
		return;
	}

	if (ABorisComputerActor* BorisComputer = Cast<ABorisComputerActor>(TargetActor))
	{
		BorisComputer->ActivateComputer();
	}
	else
	{
		static const FName ActivateFunctionName(TEXT("ActivateComputer"));
		UFunction* ActivateFunction = TargetActor->FindFunction(ActivateFunctionName);

		if (ActivateFunction)
		{
			TargetActor->ProcessEvent(ActivateFunction, nullptr);
			UE_LOG(LogTemp, Log, TEXT("BORIS: Computer activated"));
		}
		else
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("BORIS: ComputerTarget %s has no ActivateComputer function"),
				*GetNameSafe(TargetActor)
			);
			return;
		}
	}

	CompleteBorisMission();
}

EBorisMissionState ABorisCharacter::GetCurrentMissionState() const
{
	return CurrentMissionState;
}

bool ABorisCharacter::IsMissionCompleted() const
{
	return bMissionCompleted;
}

UNPCHealthComponent* ABorisCharacter::GetHealthComponent() const
{
	return HealthComponent;
}

void ABorisCharacter::HandleDamageTaken(float DamageAmount)
{
	if (IsDead() || DamageAmount <= 0.0f)
	{
		return;
	}

	if (CurrentMissionState != EBorisMissionState::WaitingAtPointA)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("BORIS: Took damage at Point A"));
	StartHurtReaction();
}

void ABorisCharacter::HandleDeath()
{
	if (bDeathHandled)
	{
		return;
	}

	bDeathHandled = true;
	StopMissionMovement();
	SetMissionState(EBorisMissionState::Dead);

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->DisableMovement();
	}

	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}

	if (bMissionCompleted)
	{
		UE_LOG(LogTemp, Log, TEXT("BORIS: Died after mission completion"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("BORIS: Died before mission completion"));
	OnBorisDiedBeforeMissionComplete.Broadcast(this);
}

void ABorisCharacter::StartHandsUp()
{
	StopMissionMovement();
	SetMissionState(EBorisMissionState::HandsUp);

	UE_LOG(LogTemp, Log, TEXT("BORIS: Hands up started"));
	OnHandsUpStarted.Broadcast(this);

	if (HandsUpMontage)
	{
		PlayAnimMontage(HandsUpMontage);
	}
}

void ABorisCharacter::MoveToPointA()
{
	if (!PointA)
	{
		UE_LOG(LogTemp, Warning, TEXT("BORIS: Cannot move to Point A because PointA is not assigned"));
		return;
	}

	AAIController* AIController = Cast<AAIController>(GetController());

	if (!AIController)
	{
		UE_LOG(LogTemp, Warning, TEXT("BORIS: Cannot move to Point A because Boris has no AIController"));
		return;
	}

	SetMissionState(EBorisMissionState::MovingToPointA);
	UE_LOG(LogTemp, Log, TEXT("BORIS: Moving to Point A"));
	AIController->MoveToActor(PointA, MoveAcceptanceRadius, true, true, true);
	StartMoveArrivalCheck();
}

void ABorisCharacter::EnterWaitingAtPointA()
{
	StopMissionMovement();
	SetMissionState(EBorisMissionState::WaitingAtPointA);

	UE_LOG(LogTemp, Log, TEXT("BORIS: Reached Point A"));
	OnReachedPointA.Broadcast(this);

	UE_LOG(LogTemp, Log, TEXT("BORIS: Waiting to be provoked"));
}

void ABorisCharacter::StartHurtReaction()
{
	StopMissionMovement();
	SetMissionState(EBorisMissionState::HurtReacting);

	UE_LOG(LogTemp, Log, TEXT("BORIS: Hurt reaction started"));
	OnBorisProvoked.Broadcast(this);

	if (HurtMontage)
	{
		PlayAnimMontage(HurtMontage);
	}
}

void ABorisCharacter::MoveToComputer()
{
	if (!ComputerTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("BORIS: Cannot move to Computer because ComputerTarget is not assigned"));
		return;
	}

	AAIController* AIController = Cast<AAIController>(GetController());

	if (!AIController)
	{
		UE_LOG(LogTemp, Warning, TEXT("BORIS: Cannot move to Computer because Boris has no AIController"));
		return;
	}

	SetMissionState(EBorisMissionState::MovingToComputer);
	UE_LOG(LogTemp, Log, TEXT("BORIS: Moving to Computer"));
	AIController->MoveToActor(ComputerTarget, MoveAcceptanceRadius, true, true, true);
	StartMoveArrivalCheck();
}

void ABorisCharacter::StartComputerActivation()
{
	StopMissionMovement();
	FaceActor(ComputerTarget);
	SetMissionState(EBorisMissionState::ActivatingComputer);

	UE_LOG(LogTemp, Log, TEXT("BORIS: Reached Computer"));
	OnReachedComputer.Broadcast(this);

	UE_LOG(LogTemp, Log, TEXT("BORIS: Activation animation started"));
	OnComputerActivationStarted.Broadcast(this);

	if (ActivateComputerMontage)
	{
		PlayAnimMontage(ActivateComputerMontage);
	}
}

void ABorisCharacter::CompleteBorisMission()
{
	if (bMissionCompleted)
	{
		return;
	}

	bMissionCompleted = true;
	SetMissionState(EBorisMissionState::Completed);

	UE_LOG(LogTemp, Log, TEXT("BORIS: Mission complete"));
	BroadcastBorisMissionCompletedEvent();
	OnBorisMissionCompleted.Broadcast(this);
}

void ABorisCharacter::StopMissionMovement()
{
	StopMoveArrivalCheck();

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	if (AController* CurrentController = GetController())
	{
		CurrentController->StopMovement();
	}
}

void ABorisCharacter::FaceActor(AActor* TargetActor)
{
	if (!TargetActor)
	{
		return;
	}

	const FVector ToTarget = TargetActor->GetActorLocation() - GetActorLocation();

	if (ToTarget.IsNearlyZero())
	{
		return;
	}

	const FRotator LookAtRotation = ToTarget.Rotation();
	SetActorRotation(FRotator(0.0f, LookAtRotation.Yaw, 0.0f));
}

void ABorisCharacter::SetMissionState(EBorisMissionState NewState)
{
	CurrentMissionState = NewState;
}

void ABorisCharacter::BroadcastBorisMissionCompletedEvent()
{
	if (!bBroadcastMissionEventOnCompletion)
	{
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	UGameplayMissionSubsystem* MissionSubsystem = World->GetSubsystem<UGameplayMissionSubsystem>();

	if (!MissionSubsystem)
	{
		return;
	}

	FMissionEventData EventData;
	EventData.EventTag = MissionCompletedEventTag;
	EventData.Instigator = this;
	EventData.Target = ComputerTarget;
	EventData.ContextId = MissionCompletedContextId;
	EventData.Amount = 1;

	MissionSubsystem->BroadcastMissionEvent(EventData);
}

bool ABorisCharacter::CanStartMissionProgression() const
{
	return CurrentMissionState == EBorisMissionState::Idle && !bMissionCompleted && !IsDead();
}

bool ABorisCharacter::IsDead() const
{
	return bDeathHandled || (HealthComponent && HealthComponent->IsDead());
}

void ABorisCharacter::CheckMoveArrival()
{
	if (IsDead())
	{
		StopMoveArrivalCheck();
		return;
	}

	AActor* MoveTarget = GetCurrentMoveTarget();

	if (!MoveTarget)
	{
		StopMoveArrivalCheck();
		return;
	}

	const float AcceptanceRadiusSquared = FMath::Square(MoveAcceptanceRadius);
	const float DistanceSquared = FVector::DistSquared(GetActorLocation(), MoveTarget->GetActorLocation());

	if (DistanceSquared > AcceptanceRadiusSquared)
	{
		return;
	}

	StopMoveArrivalCheck();

	if (CurrentMissionState == EBorisMissionState::MovingToPointA)
	{
		EnterWaitingAtPointA();
		return;
	}

	if (CurrentMissionState == EBorisMissionState::MovingToComputer)
	{
		StartComputerActivation();
	}
}

void ABorisCharacter::StartMoveArrivalCheck()
{
	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(MoveArrivalCheckTimer);
	World->GetTimerManager().SetTimer(
		MoveArrivalCheckTimer,
		this,
		&ABorisCharacter::CheckMoveArrival,
		0.1f,
		true
	);
}

void ABorisCharacter::StopMoveArrivalCheck()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MoveArrivalCheckTimer);
	}
}

AActor* ABorisCharacter::GetCurrentMoveTarget() const
{
	if (CurrentMissionState == EBorisMissionState::MovingToPointA)
	{
		return PointA;
	}

	if (CurrentMissionState == EBorisMissionState::MovingToComputer)
	{
		return ComputerTarget;
	}

	return nullptr;
}
