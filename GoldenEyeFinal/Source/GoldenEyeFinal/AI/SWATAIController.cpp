#include "SWATAIController.h"

#include "../Characters/SWATEnemyCharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"
#include "TimerManager.h"

namespace SWATBlackboardKeys
{
	const FName TargetActor(TEXT("TargetActor"));
	const FName LastKnownLocation(TEXT("LastKnownLocation"));
	const FName LastHeardLocation(TEXT("LastHeardLocation"));
	const FName HasLineOfSight(TEXT("HasLineOfSight"));
	const FName ShouldInvestigate(TEXT("ShouldInvestigate"));
	const FName IsDead(TEXT("IsDead"));
	const FName IsHitReacting(TEXT("IsHitReacting"));
	const FName IsInCombat(TEXT("IsInCombat"));
	const FName HomeLocation(TEXT("HomeLocation"));
	const FName IsSearching(TEXT("IsSearching"));
}

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
	BindControlledSWAT(InPawn);

	if (InPawn)
	{
		HomeLocation = InPawn->GetActorLocation();
	}

	if (BehaviorTreeAsset)
	{
		RunBehaviorTree(BehaviorTreeAsset);
	}

	SyncBlackboard();
	StartPerceptionDebug();
}

void ASWATAIController::OnUnPossess()
{
	StopPerceptionDebug();
	UnbindControlledSWAT();

	Super::OnUnPossess();
}

void ASWATAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopPerceptionDebug();
	UnbindControlledSWAT();

	Super::EndPlay(EndPlayReason);
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

void ASWATAIController::CompleteInvestigation()
{
	bShouldInvestigate = false;
	SyncPerceptionBlackboard();
}

void ASWATAIController::CompleteSearch()
{
	SetIsSearching(false);

	if (bHasLineOfSight)
	{
		return;
	}

	TargetActor = nullptr;

	if (ControlledSWAT)
	{
		ControlledSWAT->SetInCombat(false);
	}

	SyncBlackboard();
}

void ASWATAIController::SetIsSearching(bool bNewSearching)
{
	if (bIsSearching == bNewSearching)
	{
		SyncPerceptionBlackboard();
		return;
	}

	bIsSearching = bNewSearching;
	SyncPerceptionBlackboard();
}

void ASWATAIController::HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		HandleSightStimulus(Actor, Stimulus);
		SyncBlackboard();
		return;
	}

	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		HandleHearingStimulus(Actor, Stimulus);
		SyncBlackboard();
	}
}

void ASWATAIController::HandleControlledSWATStateChanged()
{
	if (ControlledSWAT && ControlledSWAT->IsDead())
	{
		StopMovement();
	}

	SyncSWATStateBlackboard();
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
		SetIsSearching(false);
		LastKnownLocation = Actor->GetActorLocation();
		bHasValidLastKnownLocation = true;

		if (ControlledSWAT)
		{
			ControlledSWAT->SetInCombat(true);
		}

#if ENABLE_DRAW_DEBUG
		if (bDebugPerception)
		{
			if (UWorld* World = GetWorld())
			{
				DrawDebugSphere(
					World,
					LastKnownLocation,
					DebugSphereRadius,
					12,
					FColor::Green,
					false,
					2.0f
				);
			}

			UE_LOG(
				LogTemp,
				Log,
				TEXT("[SWAT Sight Gained] Actor=%s Location=%s"),
				*GetNameSafe(Actor),
				*LastKnownLocation.ToCompactString()
			);
		}
#endif
		return;
	}

	bHasLineOfSight = false;

#if ENABLE_DRAW_DEBUG
	if (bDebugPerception)
	{
		if (UWorld* World = GetWorld())
		{
			DrawDebugSphere(
				World,
				LastKnownLocation,
				DebugSphereRadius,
				12,
				FColor::Red,
				false,
				3.0f
			);
		}

		UE_LOG(
			LogTemp,
			Log,
			TEXT("[SWAT Sight Lost] Actor=%s LastKnown=%s"),
			*GetNameSafe(Actor),
			bHasValidLastKnownLocation
				? *LastKnownLocation.ToCompactString()
				: TEXT("Invalid")
		);
	}
#endif
}

void ASWATAIController::HandleHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus)
{
	if (!Stimulus.WasSuccessfullySensed())
	{
		return;
	}

	LastHeardLocation = Stimulus.StimulusLocation;
	bHasValidLastHeardLocation = true;
	bShouldInvestigate = true;

	if (!bHasLineOfSight)
	{
		SetIsSearching(true);
	}

#if ENABLE_DRAW_DEBUG
	if (bDebugPerception)
	{
		if (UWorld* World = GetWorld())
		{
			DrawDebugSphere(
				World,
				LastHeardLocation,
				DebugSphereRadius,
				12,
				FColor::Yellow,
				false,
				3.0f
			);
		}

		UE_LOG(
			LogTemp,
			Log,
			TEXT("[SWAT Heard] Instigator=%s Location=%s"),
			*GetNameSafe(Actor),
			*LastHeardLocation.ToCompactString()
		);
	}
#endif
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

void ASWATAIController::BindControlledSWAT(APawn* InPawn)
{
	UnbindControlledSWAT();

	ControlledSWAT = Cast<ASWATEnemyCharacter>(InPawn);

	if (!ControlledSWAT)
	{
		return;
	}

	ControlledSWAT->OnSWATStateChanged.AddDynamic(
		this,
		&ASWATAIController::HandleControlledSWATStateChanged
	);
}

void ASWATAIController::UnbindControlledSWAT()
{
	if (!ControlledSWAT)
	{
		return;
	}

	ControlledSWAT->OnSWATStateChanged.RemoveDynamic(
		this,
		&ASWATAIController::HandleControlledSWATStateChanged
	);

	ControlledSWAT = nullptr;
}

void ASWATAIController::SyncBlackboard()
{
	SyncPerceptionBlackboard();
	SyncSWATStateBlackboard();
}

void ASWATAIController::SyncPerceptionBlackboard()
{
	UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();

	if (!BlackboardComponent)
	{
		return;
	}

	BlackboardComponent->SetValueAsObject(
		SWATBlackboardKeys::TargetActor,
		TargetActor
	);

	if (bHasValidLastKnownLocation)
	{
		BlackboardComponent->SetValueAsVector(
			SWATBlackboardKeys::LastKnownLocation,
			LastKnownLocation
		);
	}

	if (bHasValidLastHeardLocation)
	{
		BlackboardComponent->SetValueAsVector(
			SWATBlackboardKeys::LastHeardLocation,
			LastHeardLocation
		);
	}

	BlackboardComponent->SetValueAsBool(
		SWATBlackboardKeys::HasLineOfSight,
		bHasLineOfSight
	);

	BlackboardComponent->SetValueAsBool(
		SWATBlackboardKeys::ShouldInvestigate,
		bShouldInvestigate
	);

	BlackboardComponent->SetValueAsVector(
		SWATBlackboardKeys::HomeLocation,
		HomeLocation
	);

	BlackboardComponent->SetValueAsBool(
		SWATBlackboardKeys::IsSearching,
		bIsSearching
	);
}

void ASWATAIController::SyncSWATStateBlackboard()
{
	UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();

	if (!BlackboardComponent)
	{
		return;
	}

	const bool bSWATIsDead =
		ControlledSWAT ? ControlledSWAT->IsDead() : false;
	const bool bSWATIsHitReacting =
		ControlledSWAT ? ControlledSWAT->IsHitReacting() : false;
	const bool bSWATIsInCombat =
		ControlledSWAT ? ControlledSWAT->IsInCombat() : false;

	BlackboardComponent->SetValueAsBool(
		SWATBlackboardKeys::IsDead,
		bSWATIsDead
	);

	BlackboardComponent->SetValueAsBool(
		SWATBlackboardKeys::IsHitReacting,
		bSWATIsHitReacting
	);

	BlackboardComponent->SetValueAsBool(
		SWATBlackboardKeys::IsInCombat,
		bSWATIsInCombat
	);
}

void ASWATAIController::StartPerceptionDebug()
{
#if ENABLE_DRAW_DEBUG
	if (!bDebugPerception || !GetPawn())
	{
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	FTimerManager& TimerManager = World->GetTimerManager();

	if (TimerManager.IsTimerActive(PerceptionDebugTimerHandle))
	{
		return;
	}

	TimerManager.SetTimer(
		PerceptionDebugTimerHandle,
		this,
		&ASWATAIController::DrawPerceptionDebug,
		PerceptionDebugInterval,
		true
	);

	UE_LOG(LogTemp, Log, TEXT("[SWAT Debug] Started"));
#endif
}

void ASWATAIController::StopPerceptionDebug()
{
#if ENABLE_DRAW_DEBUG
	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	FTimerManager& TimerManager = World->GetTimerManager();
	const bool bWasActive =
		TimerManager.IsTimerActive(PerceptionDebugTimerHandle);

	TimerManager.ClearTimer(PerceptionDebugTimerHandle);

	if (bDebugPerception && bWasActive)
	{
		UE_LOG(LogTemp, Log, TEXT("[SWAT Debug] Stopped"));
	}
#endif
}

void ASWATAIController::DrawPerceptionDebug()
{
#if ENABLE_DRAW_DEBUG
	if (!bDebugPerception || !GetWorld() || !GetPawn())
	{
		return;
	}

	DrawSightDebug();
	DrawLastKnownLocationDebug();
	DrawLastHeardLocationDebug();
	DrawPerceptionStateText();
#endif
}

void ASWATAIController::DrawSightDebug() const
{
#if ENABLE_DRAW_DEBUG
	UWorld* World = GetWorld();
	const APawn* ControlledPawn = GetPawn();

	if (!World || !ControlledPawn)
	{
		return;
	}

	FVector EyeLocation;
	FRotator EyeRotation;
	ControlledPawn->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	const float Lifetime =
		FMath::Max(DebugDrawDuration, PerceptionDebugInterval + 0.01f);

	const FVector Forward = EyeRotation.Vector();
	const FVector SightEnd = EyeLocation + (Forward * SightRadius);
	const FVector LeftBoundary =
		Forward.RotateAngleAxis(
			PeripheralVisionHalfAngleDegrees,
			FVector::UpVector
		);
	const FVector RightBoundary =
		Forward.RotateAngleAxis(
			-PeripheralVisionHalfAngleDegrees,
			FVector::UpVector
		);

	DrawDebugDirectionalArrow(
		World,
		EyeLocation,
		SightEnd,
		DebugSphereRadius,
		FColor::White,
		false,
		Lifetime,
		0,
		DebugLineThickness
	);

	DrawDebugLine(
		World,
		EyeLocation,
		EyeLocation + (LeftBoundary * SightRadius),
		FColor::White,
		false,
		Lifetime,
		0,
		DebugLineThickness
	);

	DrawDebugLine(
		World,
		EyeLocation,
		EyeLocation + (RightBoundary * SightRadius),
		FColor::White,
		false,
		Lifetime,
		0,
		DebugLineThickness
	);

	if (TargetActor && bHasLineOfSight)
	{
		const FVector TargetLocation = TargetActor->GetActorLocation();

		DrawDebugLine(
			World,
			EyeLocation,
			TargetLocation,
			FColor::Green,
			false,
			Lifetime,
			0,
			DebugLineThickness
		);

		DrawDebugSphere(
			World,
			TargetLocation,
			DebugSphereRadius,
			12,
			FColor::Green,
			false,
			Lifetime
		);

		DrawDebugDirectionalArrow(
			World,
			EyeLocation,
			TargetLocation,
			DebugSphereRadius,
			FColor::Green,
			false,
			Lifetime,
			0,
			DebugLineThickness
		);
	}
	else if (TargetActor && bHasValidLastKnownLocation)
	{
		DrawDebugLine(
			World,
			EyeLocation,
			LastKnownLocation,
			FColor::Red,
			false,
			Lifetime,
			0,
			DebugLineThickness
		);
	}
#endif
}

void ASWATAIController::DrawLastKnownLocationDebug() const
{
#if ENABLE_DRAW_DEBUG
	if (!bHasValidLastKnownLocation)
	{
		return;
	}

	UWorld* World = GetWorld();
	const APawn* ControlledPawn = GetPawn();

	if (!World || !ControlledPawn)
	{
		return;
	}

	const float Lifetime =
		FMath::Max(DebugDrawDuration, PerceptionDebugInterval + 0.01f);
	const FVector MarkerTop =
		LastKnownLocation + FVector(0.0f, 0.0f, DebugTextHeight);

	DrawDebugSphere(
		World,
		LastKnownLocation,
		DebugSphereRadius,
		12,
		FColor::Red,
		false,
		Lifetime
	);

	DrawDebugLine(
		World,
		LastKnownLocation,
		MarkerTop,
		FColor::Red,
		false,
		Lifetime,
		0,
		DebugLineThickness
	);

	DrawDebugString(
		World,
		MarkerTop,
		TEXT("LAST KNOWN"),
		nullptr,
		FColor::Red,
		Lifetime,
		false
	);

	DrawDebugDirectionalArrow(
		World,
		ControlledPawn->GetActorLocation(),
		LastKnownLocation,
		DebugSphereRadius,
		FColor::Red,
		false,
		Lifetime,
		0,
		DebugLineThickness
	);
#endif
}

void ASWATAIController::DrawLastHeardLocationDebug() const
{
#if ENABLE_DRAW_DEBUG
	if (!bHasValidLastHeardLocation)
	{
		return;
	}

	UWorld* World = GetWorld();
	const APawn* ControlledPawn = GetPawn();

	if (!World || !ControlledPawn)
	{
		return;
	}

	const float Lifetime =
		FMath::Max(DebugDrawDuration, PerceptionDebugInterval + 0.01f);
	const FVector MarkerTop =
		LastHeardLocation + FVector(0.0f, 0.0f, DebugTextHeight);

	DrawDebugSphere(
		World,
		LastHeardLocation,
		DebugSphereRadius,
		12,
		FColor::Yellow,
		false,
		Lifetime
	);

	DrawDebugLine(
		World,
		LastHeardLocation,
		MarkerTop,
		FColor::Yellow,
		false,
		Lifetime,
		0,
		DebugLineThickness
	);

	DrawDebugString(
		World,
		MarkerTop,
		TEXT("LAST HEARD"),
		nullptr,
		FColor::Yellow,
		Lifetime,
		false
	);

	DrawDebugDirectionalArrow(
		World,
		ControlledPawn->GetActorLocation(),
		LastHeardLocation,
		DebugSphereRadius,
		FColor::Yellow,
		false,
		Lifetime,
		0,
		DebugLineThickness
	);
#endif
}

void ASWATAIController::DrawPerceptionStateText() const
{
#if ENABLE_DRAW_DEBUG
	UWorld* World = GetWorld();
	const APawn* ControlledPawn = GetPawn();

	if (!World || !ControlledPawn)
	{
		return;
	}

	const float Lifetime =
		FMath::Max(DebugDrawDuration, PerceptionDebugInterval + 0.01f);
	const FVector TextLocation =
		ControlledPawn->GetActorLocation() +
		FVector(0.0f, 0.0f, DebugTextHeight);
	const FString TargetName =
		TargetActor ? TargetActor->GetName() : TEXT("None");
	const FString LastKnownText =
		bHasValidLastKnownLocation
			? LastKnownLocation.ToCompactString()
			: TEXT("Invalid");
	const FString LastHeardText =
		bHasValidLastHeardLocation
			? LastHeardLocation.ToCompactString()
			: TEXT("Invalid");
	const FString DebugText = FString::Printf(
		TEXT("Target: %s\nHasLineOfSight: %s\nShouldInvestigate: %s\nLastKnownLocation: %s\nLastHeardLocation: %s"),
		*TargetName,
		bHasLineOfSight ? TEXT("TRUE") : TEXT("FALSE"),
		bShouldInvestigate ? TEXT("TRUE") : TEXT("FALSE"),
		*LastKnownText,
		*LastHeardText
	);

	DrawDebugString(
		World,
		TextLocation,
		DebugText,
		nullptr,
		FColor::White,
		Lifetime,
		false
	);
#endif
}
