#include "CopyOpportunity.h"

#include "../Characters/JamesBondCharacter.h"
#include "../Mission/GameplayMissionSubsystem.h"
#include "../Mission/MissionTypes.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ACopyOpportunity::ACopyOpportunity()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	SetRootComponent(TriggerBox);

	TriggerBox->SetBoxExtent(FVector(120.0f, 120.0f, 100.0f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
}

void ACopyOpportunity::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(
			this,
			&ACopyOpportunity::HandleTriggerBeginOverlap
		);

		TriggerBox->OnComponentEndOverlap.AddDynamic(
			this,
			&ACopyOpportunity::HandleTriggerEndOverlap
		);
	}
}

void ACopyOpportunity::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearCopyTimer();

	Super::EndPlay(EndPlayReason);
}

void ACopyOpportunity::TryStartCopy()
{
	if (!bPlayerInside)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("%s copy attempt rejected because Bond is outside."),
			*GetName()
		);
		return;
	}

	if (bCopyInProgress)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("%s copy attempt rejected because copy is already running."),
			*GetName()
		);
		return;
	}

	if (bCopyCompleted)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("%s copy attempt rejected because copy is already completed."),
			*GetName()
		);
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s copy attempt rejected because no world was available."), *GetName());
		return;
	}

	bCopyInProgress = true;
	CurrentCopyProgress = 0.0f;
	CopyElapsedTime = 0.0f;
	NextProgressLogMilestone = 0.25f;

	UE_LOG(LogTemp, Log, TEXT("%s copy started."), *GetName());

	OnCopyProgressChanged.Broadcast(CurrentCopyProgress);
	OnCopyStarted.Broadcast();

	World->GetTimerManager().SetTimer(
		CopyProgressTimer,
		this,
		&ACopyOpportunity::UpdateCopyProgress,
		0.05f,
		true
	);
}

float ACopyOpportunity::GetCurrentCopyProgress() const
{
	return CurrentCopyProgress;
}

bool ACopyOpportunity::IsPlayerInside() const
{
	return bPlayerInside;
}

bool ACopyOpportunity::IsCopyInProgress() const
{
	return bCopyInProgress;
}

bool ACopyOpportunity::IsCopyCompleted() const
{
	return bCopyCompleted;
}

void ACopyOpportunity::HandleTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	AJamesBondCharacter* Bond = Cast<AJamesBondCharacter>(OtherActor);

	if (!Bond)
	{
		return;
	}

	OverlappingBond = Bond;
	bPlayerInside = true;
	Bond->SetCopyOpportunity(this);

	UE_LOG(LogTemp, Log, TEXT("Bond entered copy zone: %s."), *GetName());

	if (!bCopyCompleted)
	{
		UE_LOG(LogTemp, Log, TEXT("%s copy became available."), *GetName());
		OnCopyAvailable.Broadcast();
	}
}

void ACopyOpportunity::HandleTriggerEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex
)
{
	AJamesBondCharacter* Bond = Cast<AJamesBondCharacter>(OtherActor);

	if (!Bond || Bond != OverlappingBond)
	{
		return;
	}

	bPlayerInside = false;
	Bond->ClearCopyOpportunity(this);

	UE_LOG(LogTemp, Log, TEXT("Bond left copy zone: %s."), *GetName());
	OnCopyUnavailable.Broadcast();

	if (bCopyInProgress)
	{
		CancelCopy();
	}

	OverlappingBond = nullptr;
}

void ACopyOpportunity::UpdateCopyProgress()
{
	if (!bCopyInProgress)
	{
		ClearCopyTimer();
		return;
	}

	CopyElapsedTime += 0.05f;
	CurrentCopyProgress = FMath::Clamp(CopyElapsedTime / FMath::Max(CopyDuration, KINDA_SMALL_NUMBER), 0.0f, 1.0f);

	OnCopyProgressChanged.Broadcast(CurrentCopyProgress);

	if (CurrentCopyProgress >= NextProgressLogMilestone && CurrentCopyProgress < 1.0f)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("%s copy progress: %.0f%%."),
			*GetName(),
			CurrentCopyProgress * 100.0f
		);

		NextProgressLogMilestone += 0.25f;
	}

	if (CurrentCopyProgress >= 1.0f)
	{
		CompleteCopy();
	}
}

void ACopyOpportunity::CompleteCopy()
{
	if (bCopyCompleted)
	{
		return;
	}

	ClearCopyTimer();

	CurrentCopyProgress = 1.0f;
	bCopyInProgress = false;
	bCopyCompleted = true;

	OnCopyProgressChanged.Broadcast(1.0f);

	UE_LOG(LogTemp, Log, TEXT("%s copy completed."), *GetName());
	OnCopyCompleted.Broadcast();
	BroadcastCopyMissionEvent();
}

void ACopyOpportunity::CancelCopy()
{
	if (!bCopyInProgress)
	{
		return;
	}

	ClearCopyTimer();

	CurrentCopyProgress = 0.0f;
	CopyElapsedTime = 0.0f;
	NextProgressLogMilestone = 0.25f;
	bCopyInProgress = false;

	UE_LOG(LogTemp, Log, TEXT("%s copy cancelled. Progress reset to 0."), *GetName());

	OnCopyProgressChanged.Broadcast(CurrentCopyProgress);
	OnCopyCancelled.Broadcast();
}

void ACopyOpportunity::ClearCopyTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CopyProgressTimer);
	}
}

void ACopyOpportunity::BroadcastCopyMissionEvent()
{
	if (!bBroadcastMissionEventOnCopyCompleted)
	{
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission] Copy event failed: missing world CopyOpportunity=%s"), *GetName());
		return;
	}

	UGameplayMissionSubsystem* MissionSubsystem = World->GetSubsystem<UGameplayMissionSubsystem>();

	if (!MissionSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission] Copy event failed: missing mission subsystem CopyOpportunity=%s"), *GetName());
		return;
	}

	FMissionEventData EventData;
	EventData.EventTag = MissionEventTag;
	EventData.Instigator = OverlappingBond;
	EventData.Target = this;
	EventData.ContextId = MissionContextId;
	EventData.Amount = 1;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[Mission] Copy completed event CopyOpportunity=%s Event=%s Context=%s"),
		*GetName(),
		*MissionEventTag.ToString(),
		*MissionContextId.ToString()
	);

	MissionSubsystem->BroadcastMissionEvent(EventData);
}
