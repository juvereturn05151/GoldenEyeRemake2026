#include "BorisCharacter.h"

#include "../AI/BorisAIController.h"
#include "../Actors/BorisComputerActor.h"
#include "../Components/NPCHealthComponent.h"
#include "../Mission/GameplayMissionSubsystem.h"
#include "../Mission/MissionTypes.h"
#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

namespace
{
	const TCHAR* GetBorisMissionStateLogName(EBorisMissionState State)
	{
		switch (State)
		{
		case EBorisMissionState::Idle:
			return TEXT("Idle");
		case EBorisMissionState::HandsUp:
			return TEXT("HandsUp");
		case EBorisMissionState::MovingToPointA:
			return TEXT("MovingToPointA");
		case EBorisMissionState::WaitingAtPointA:
			return TEXT("WaitingAtPointA");
		case EBorisMissionState::HurtReacting:
			return TEXT("HurtReacting");
		case EBorisMissionState::MovingToComputer:
			return TEXT("MovingToComputer");
		case EBorisMissionState::ActivatingComputer:
			return TEXT("ActivatingComputer");
		case EBorisMissionState::Completed:
			return TEXT("Completed");
		case EBorisMissionState::Dead:
			return TEXT("Dead");
		default:
			return TEXT("Unknown");
		}
	}
}

ABorisCharacter::ABorisCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	bUseControllerRotationYaw = true;
	AIControllerClass = ABorisAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	HealthComponent = CreateDefaultSubobject<UNPCHealthComponent>(TEXT("HealthComponent"));

	WalkingHumAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WalkingHumAudioComponent"));
	WalkingHumAudioComponent->SetupAttachment(GetRootComponent());
	WalkingHumAudioComponent->bAutoActivate = false;

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Capsule->SetCollisionObjectType(ECC_Pawn);
		Capsule->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}

	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->bOrientRotationToMovement = true;
		MovementComponent->bUseControllerDesiredRotation = false;
		MovementComponent->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
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
	StopWalkingHum();

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
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("BORIS: Damage ignored Damage=%.2f IsDead=%s HasHealthComponent=%s State=%s"),
			DamageAmount,
			IsDead() ? TEXT("true") : TEXT("false"),
			HealthComponent ? TEXT("true") : TEXT("false"),
			GetBorisMissionStateLogName(CurrentMissionState)
		);
		return 0.0f;
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("BORIS: Damage received Damage=%.2f State=%s"),
		DamageAmount,
		GetBorisMissionStateLogName(CurrentMissionState)
	);

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
	PlayBorisSound(IAmScaredSound);
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

	ABorisComputerActor* ComputerToActivate = GetBorisComputerToActivate();

	if (!ComputerToActivate)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("BORIS: Cannot activate computer because BorisComputer is not assigned and ComputerTarget is not a BorisComputerActor")
		);
		return;
	}

	ComputerToActivate->ActivateComputer();
	PlayBorisSound(ActivateCompleteSound);

	CompleteBorisMission();
}

void ABorisCharacter::NotifyMissionMoveCompleted(bool bSucceeded)
{
	if (IsDead())
	{
		return;
	}

	if (!bSucceeded)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("BORIS: Move completed unsuccessfully State=%s"),
			GetBorisMissionStateLogName(CurrentMissionState)
		);
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
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("BORIS: Damage event ignored before hurt flow Damage=%.2f State=%s IsDead=%s"),
			DamageAmount,
			GetBorisMissionStateLogName(CurrentMissionState),
			IsDead() ? TEXT("true") : TEXT("false")
		);
		return;
	}

	if (!CanDamageTriggerHurtFlow())
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("BORIS: Damage did not trigger hurt flow because State=%s"),
			GetBorisMissionStateLogName(CurrentMissionState)
		);
		return;
	}

	if (HealthComponent && HealthComponent->GetCurrentHealth() <= 0.0f)
	{
		UE_LOG(LogTemp, Log, TEXT("BORIS: Damage did not trigger hurt flow because Boris is dying."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("BORIS: Took damage and will move to Computer after hurt reaction"));
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
	StopWalkingHum();
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

	StopCurrentMontageImmediately();
	SetMissionState(EBorisMissionState::MovingToPointA);
	SetMovementRotationMode(true);
	StartWalkingHum();
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
	PlayBorisSound(AnnoyingLineSound);
}

void ABorisCharacter::StartHurtReaction()
{
	StopMissionMovement();
	SetMissionState(EBorisMissionState::HurtReacting);

	UE_LOG(LogTemp, Log, TEXT("BORIS: Hurt reaction started"));
	PlayBorisSound(HurtSound);
	OnBorisProvoked.Broadcast(this);

	if (HurtMontage)
	{
		const float PlayLength = PlayAnimMontage(HurtMontage);

		if (PlayLength <= 0.0f)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("BORIS: Hurt montage was assigned but did not play. Check ABP_Boris Slot setup and montage skeleton.")
			);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BORIS: HurtMontage is not assigned on this Boris instance."));
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

	StopCurrentMontageImmediately();
	SetMissionState(EBorisMissionState::MovingToComputer);
	SetMovementRotationMode(true);
	StartWalkingHum();
	UE_LOG(LogTemp, Log, TEXT("BORIS: Moving to Computer"));
	AIController->MoveToActor(ComputerTarget, MoveAcceptanceRadius, true, true, true);
	StartMoveArrivalCheck();
}

void ABorisCharacter::StartComputerActivation()
{
	StopMissionMovement();
	FaceActor(GetBorisComputerToActivate() ? static_cast<AActor*>(GetBorisComputerToActivate()) : ComputerTarget.Get());
	SetMissionState(EBorisMissionState::ActivatingComputer);

	UE_LOG(LogTemp, Log, TEXT("BORIS: Reached Computer"));
	OnReachedComputer.Broadcast(this);

	UE_LOG(LogTemp, Log, TEXT("BORIS: Activation animation started"));
	PlayBorisSound(ActivatingSound);
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
	StopWalkingHum();
	SetMovementRotationMode(false);

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	if (AController* CurrentController = GetController())
	{
		CurrentController->StopMovement();
	}
}

void ABorisCharacter::StopCurrentMontageImmediately()
{
	USkeletalMeshComponent* MeshComponent = GetMesh();

	if (!MeshComponent)
	{
		return;
	}

	UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();

	if (!AnimInstance)
	{
		return;
	}

	AnimInstance->Montage_Stop(0.0f);
}

void ABorisCharacter::PlayBorisSound(USoundBase* Sound) const
{
	if (!Sound)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
}

void ABorisCharacter::StartWalkingHum()
{
	if (!WalkingHumAudioComponent || !HumWhileWalkingSound)
	{
		return;
	}

	if (WalkingHumAudioComponent->IsPlaying())
	{
		return;
	}

	WalkingHumAudioComponent->SetSound(HumWhileWalkingSound);
	WalkingHumAudioComponent->Play();
}

void ABorisCharacter::StopWalkingHum()
{
	if (WalkingHumAudioComponent && WalkingHumAudioComponent->IsPlaying())
	{
		WalkingHumAudioComponent->Stop();
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

void ABorisCharacter::SetMovementRotationMode(bool bOrientToMovement)
{
	bUseControllerRotationYaw = !bOrientToMovement;

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->bOrientRotationToMovement = bOrientToMovement;
		MovementComponent->bUseControllerDesiredRotation = false;
	}
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
	EventData.Target = GetBorisComputerToActivate() ? static_cast<AActor*>(GetBorisComputerToActivate()) : ComputerTarget.Get();
	EventData.ContextId = MissionCompletedContextId;
	EventData.Amount = 1;

	MissionSubsystem->BroadcastMissionEvent(EventData);
}

ABorisComputerActor* ABorisCharacter::GetBorisComputerToActivate() const
{
	if (BorisComputer)
	{
		return BorisComputer;
	}

	return Cast<ABorisComputerActor>(ComputerTarget);
}

bool ABorisCharacter::CanStartMissionProgression() const
{
	return CurrentMissionState == EBorisMissionState::Idle && !bMissionCompleted && !IsDead();
}

bool ABorisCharacter::CanDamageTriggerHurtFlow() const
{
	if (bMissionCompleted || IsDead())
	{
		return false;
	}

	switch (CurrentMissionState)
	{
	case EBorisMissionState::HandsUp:
	case EBorisMissionState::MovingToPointA:
	case EBorisMissionState::WaitingAtPointA:
		return true;
	default:
		return false;
	}
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
