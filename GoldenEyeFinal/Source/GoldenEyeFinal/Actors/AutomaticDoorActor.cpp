#include "AutomaticDoorActor.h"

#include "../Characters/BorisCharacter.h"
#include "../Characters/JamesBondCharacter.h"
#include "../Characters/SWATEnemyCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

AAutomaticDoorActor::AAutomaticDoorActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(SceneRoot);
	DoorMesh->SetMobility(EComponentMobility::Movable);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(SceneRoot);
	TriggerBox->SetBoxExtent(TriggerBoxExtent);
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
}

void AAutomaticDoorActor::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerBox)
	{
		TriggerBox->SetBoxExtent(TriggerBoxExtent);
		TriggerBox->OnComponentBeginOverlap.AddDynamic(
			this,
			&AAutomaticDoorActor::HandleTriggerBeginOverlap
		);
		TriggerBox->OnComponentEndOverlap.AddDynamic(
			this,
			&AAutomaticDoorActor::HandleTriggerEndOverlap
		);
	}

	if (DoorMesh)
	{
		ClosedRelativeLocation = DoorMesh->GetRelativeLocation();
		OpenRelativeLocation = ClosedRelativeLocation + OpenOffset;
	}

	CurrentOpenAlpha = bStartOpen ? 1.0f : 0.0f;
	TargetOpenAlpha = CurrentOpenAlpha;
	bIsOpen = bStartOpen;

	if (DoorMesh)
	{
		DoorMesh->SetRelativeLocation(
			FMath::Lerp(ClosedRelativeLocation, OpenRelativeLocation, CurrentOpenAlpha)
		);
	}
}

void AAutomaticDoorActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	RemoveInvalidOverlappingUsers();
	RefreshDoorTarget();
	UpdateDoorMovement(DeltaSeconds);
}

void AAutomaticDoorActor::OpenDoor()
{
	if (bLocked)
	{
		return;
	}

	TimeSinceLastUserLeft = 0.0f;
	TargetOpenAlpha = 1.0f;

	if (!bIsOpen)
	{
		bIsOpen = true;
		PlayOpenSound();
		OnDoorOpenStarted();
	}
}

void AAutomaticDoorActor::CloseDoor()
{
	TargetOpenAlpha = 0.0f;

	if (bIsOpen)
	{
		bIsOpen = false;
		OnDoorCloseStarted();
	}
}

bool AAutomaticDoorActor::IsOpen() const
{
	return bIsOpen;
}

void AAutomaticDoorActor::SetLocked(bool bNewLocked)
{
	if (bLocked == bNewLocked)
	{
		return;
	}

	bLocked = bNewLocked;

	if (bLocked)
	{
		OverlappingUsers.Reset();
		CloseDoor();
		OnDoorLocked();
		return;
	}

	RefreshOverlappingUsersFromTrigger();
	OnDoorUnlocked();

	if (OverlappingUsers.Num() > 0)
	{
		OpenDoor();
	}
}

void AAutomaticDoorActor::LockDoor()
{
	SetLocked(true);
}

void AAutomaticDoorActor::UnlockDoor()
{
	SetLocked(false);
}

bool AAutomaticDoorActor::IsLocked() const
{
	return bLocked;
}

void AAutomaticDoorActor::HandleTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (!CanUseDoor(OtherActor))
	{
		return;
	}

	OverlappingUsers.Add(OtherActor);
	OpenDoor();
}

void AAutomaticDoorActor::HandleTriggerEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex
)
{
	if (!OtherActor)
	{
		return;
	}

	OverlappingUsers.Remove(OtherActor);

	if (OverlappingUsers.Num() == 0)
	{
		TimeSinceLastUserLeft = 0.0f;
	}
}

void AAutomaticDoorActor::UpdateDoorMovement(float DeltaSeconds)
{
	if (!DoorMesh)
	{
		return;
	}

	const float Duration = TargetOpenAlpha > CurrentOpenAlpha
		? OpenDuration
		: CloseDuration;
	const float InterpSpeed = Duration > 0.0f
		? 1.0f / Duration
		: 1.0f;

	CurrentOpenAlpha = FMath::FInterpConstantTo(
		CurrentOpenAlpha,
		TargetOpenAlpha,
		DeltaSeconds,
		InterpSpeed
	);

	DoorMesh->SetRelativeLocation(
		FMath::Lerp(ClosedRelativeLocation, OpenRelativeLocation, CurrentOpenAlpha)
	);
}

void AAutomaticDoorActor::RefreshDoorTarget()
{
	if (bLocked)
	{
		if (bIsOpen || TargetOpenAlpha > 0.0f)
		{
			CloseDoor();
		}
		return;
	}

	if (OverlappingUsers.Num() > 0)
	{
		OpenDoor();
		return;
	}

	TimeSinceLastUserLeft += GetWorld()
		? GetWorld()->GetDeltaSeconds()
		: 0.0f;

	if (TimeSinceLastUserLeft >= CloseDelay)
	{
		CloseDoor();
	}
}

void AAutomaticDoorActor::RemoveInvalidOverlappingUsers()
{
	for (auto It = OverlappingUsers.CreateIterator(); It; ++It)
	{
		AActor* User = It->Get();

		if (!CanUseDoor(User))
		{
			It.RemoveCurrent();
		}
	}
}

void AAutomaticDoorActor::RefreshOverlappingUsersFromTrigger()
{
	OverlappingUsers.Reset();

	if (!TriggerBox)
	{
		return;
	}

	TArray<AActor*> OverlappingActors;
	TriggerBox->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (CanUseDoor(Actor))
		{
			OverlappingUsers.Add(Actor);
		}
	}
}

bool AAutomaticDoorActor::CanUseDoor(AActor* Actor) const
{
	if (bLocked || !Actor)
	{
		return false;
	}

	if (Actor->IsA<AJamesBondCharacter>())
	{
		return true;
	}

	const ASWATEnemyCharacter* SWATCharacter = Cast<ASWATEnemyCharacter>(Actor);

	if (SWATCharacter)
	{
		return !SWATCharacter->IsDead();
	}

	const ABorisCharacter* BorisCharacter = Cast<ABorisCharacter>(Actor);

	return BorisCharacter && BorisCharacter->GetCurrentMissionState() != EBorisMissionState::Dead;
}

void AAutomaticDoorActor::PlayOpenSound() const
{
	if (!OpenSound)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(
		this,
		OpenSound,
		GetActorLocation(),
		OpenSoundVolume
	);
}
