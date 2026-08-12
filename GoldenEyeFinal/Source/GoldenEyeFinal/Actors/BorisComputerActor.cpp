#include "BorisComputerActor.h"

#include "../Characters/JamesBondCharacter.h"
#include "../Mission/GameplayMissionSubsystem.h"
#include "../Mission/MissionTypes.h"
#include "../Player/BondPlayerController.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"

ABorisComputerActor::ABorisComputerActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(SceneRoot);
	TriggerBox->SetBoxExtent(FVector(120.0f, 120.0f, 100.0f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
}

void ABorisComputerActor::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(
			this,
			&ABorisComputerActor::HandleTriggerBeginOverlap
		);

		TriggerBox->OnComponentEndOverlap.AddDynamic(
			this,
			&ABorisComputerActor::HandleTriggerEndOverlap
		);
	}
}

bool ABorisComputerActor::ActivateComputer()
{
	if (bActivated)
	{
		UE_LOG(LogTemp, Log, TEXT("BORIS: Computer activation ignored because %s is already activated"), *GetNameSafe(this));
		return false;
	}

	bActivated = true;
	UE_LOG(LogTemp, Log, TEXT("BORIS: Computer activated"));
	OnComputerActivated.Broadcast(this);
	BroadcastActivationMissionEvent();

	if (OverlappingBond)
	{
		SetBondInteractionAvailable(OverlappingBond, true);
	}

	return true;
}

bool ABorisComputerActor::IsActivated() const
{
	return bActivated;
}

bool ABorisComputerActor::TryInteract(AJamesBondCharacter* Bond)
{
	if (!Bond || Bond != OverlappingBond)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s computer interaction failed: Bond is not inside the trigger."), *GetName());
		return false;
	}

	if (!IsInteractionAvailable())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("%s computer interaction failed: Activated=%s Completed=%s"),
			*GetName(),
			bActivated ? TEXT("true") : TEXT("false"),
			bInteractionCompleted ? TEXT("true") : TEXT("false")
		);
		return false;
	}

	bInteractionCompleted = true;

	UE_LOG(LogTemp, Log, TEXT("%s computer interacted by Bond."), *GetName());
	OnComputerInteracted.Broadcast(this);
	BroadcastInteractionMissionEvent(Bond);

	if (!bAllowRepeatedInteraction)
	{
		SetBondInteractionAvailable(Bond, false);
	}

	return true;
}

bool ABorisComputerActor::IsInteractionAvailable() const
{
	return bActivated && bBondInsideTrigger && (bAllowRepeatedInteraction || !bInteractionCompleted);
}

bool ABorisComputerActor::HasBondInsideTrigger() const
{
	return bBondInsideTrigger;
}

void ABorisComputerActor::HandleTriggerBeginOverlap(
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
	bBondInsideTrigger = true;

	UE_LOG(LogTemp, Log, TEXT("%s Bond entered computer trigger. Activated=%s"), *GetName(), bActivated ? TEXT("true") : TEXT("false"));

	if (IsInteractionAvailable())
	{
		SetBondInteractionAvailable(Bond, true);
	}
}

void ABorisComputerActor::BroadcastActivationMissionEvent()
{
	if (!bBroadcastMissionEventOnActivated)
	{
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission] Computer activation event failed: missing world Computer=%s"), *GetName());
		return;
	}

	UGameplayMissionSubsystem* MissionSubsystem = World->GetSubsystem<UGameplayMissionSubsystem>();

	if (!MissionSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission] Computer activation event failed: missing mission subsystem Computer=%s"), *GetName());
		return;
	}

	FMissionEventData EventData;
	EventData.EventTag = ActivationMissionEventTag;
	EventData.Instigator = this;
	EventData.Target = this;
	EventData.ContextId = ActivationMissionContextId;
	EventData.Amount = 1;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[Mission] Computer activated event Computer=%s Event=%s Context=%s"),
		*GetName(),
		*ActivationMissionEventTag.ToString(),
		*ActivationMissionContextId.ToString()
	);

	MissionSubsystem->BroadcastMissionEvent(EventData);
}

void ABorisComputerActor::BroadcastInteractionMissionEvent(AJamesBondCharacter* Bond)
{
	if (!bBroadcastMissionEventOnInteraction)
	{
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission] Computer interaction event failed: missing world Computer=%s"), *GetName());
		return;
	}

	UGameplayMissionSubsystem* MissionSubsystem = World->GetSubsystem<UGameplayMissionSubsystem>();

	if (!MissionSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission] Computer interaction event failed: missing mission subsystem Computer=%s"), *GetName());
		return;
	}

	FMissionEventData EventData;
	EventData.EventTag = InteractionMissionEventTag;
	EventData.Instigator = Bond;
	EventData.Target = this;
	EventData.ContextId = InteractionMissionContextId;
	EventData.Amount = 1;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[Mission] Computer interaction event Computer=%s Event=%s Context=%s Instigator=%s"),
		*GetName(),
		*InteractionMissionEventTag.ToString(),
		*InteractionMissionContextId.ToString(),
		*GetNameSafe(Bond)
	);

	MissionSubsystem->BroadcastMissionEvent(EventData);
}

void ABorisComputerActor::HandleTriggerEndOverlap(
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

	SetBondInteractionAvailable(Bond, false);
	bBondInsideTrigger = false;
	OverlappingBond = nullptr;

	UE_LOG(LogTemp, Log, TEXT("%s Bond left computer trigger."), *GetName());
}

void ABorisComputerActor::SetBondInteractionAvailable(AJamesBondCharacter* Bond, bool bAvailable)
{
	if (!Bond)
	{
		return;
	}

	if (bAvailable)
	{
		Bond->SetComputerInteraction(this);
		OnComputerInteractionAvailable.Broadcast(this);

		if (ABondPlayerController* BondController = Cast<ABondPlayerController>(Bond->GetController()))
		{
			BondController->ShowInteractionPrompt(InteractionPromptText);
		}

		return;
	}

	Bond->ClearComputerInteraction(this);
	OnComputerInteractionUnavailable.Broadcast(this);

	if (ABondPlayerController* BondController = Cast<ABondPlayerController>(Bond->GetController()))
	{
		BondController->HideInteractionPrompt();
	}
}
