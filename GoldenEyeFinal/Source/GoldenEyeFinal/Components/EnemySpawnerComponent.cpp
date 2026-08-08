#include "EnemySpawnerComponent.h"

#include "../AI/SWATAIController.h"
#include "../Characters/JamesBondCharacter.h"
#include "../Characters/SWATEnemyCharacter.h"
#include "AIController.h"
#include "Engine/TargetPoint.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UEnemySpawnerComponent::UEnemySpawnerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	InitBoxExtent(FVector(150.0f, 150.0f, 100.0f));
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionObjectType(ECC_WorldDynamic);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SetGenerateOverlapEvents(true);
}

void UEnemySpawnerComponent::BeginPlay()
{
	Super::BeginPlay();

	OnComponentBeginOverlap.AddUniqueDynamic(
		this,
		&UEnemySpawnerComponent::HandleBeginOverlap
	);
	OnComponentEndOverlap.AddUniqueDynamic(
		this,
		&UEnemySpawnerComponent::HandleEndOverlap
	);
}

void UEnemySpawnerComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!CurrentBondActor || !CanActivate())
	{
		return;
	}

	BondInsideTime += DeltaTime;

	if (BondInsideTime < RequiredBondInsideSeconds)
	{
		return;
	}

	ActivateSpawner();
	BondInsideTime = 0.0f;
}

void UEnemySpawnerComponent::ResetSpawner()
{
	ActivationCount = 0;
	BondInsideTime = 0.0f;
	NextSpawnPointIndex = 0;
	CurrentBondActor = nullptr;
}

int32 UEnemySpawnerComponent::GetActivationCount() const
{
	return ActivationCount;
}

float UEnemySpawnerComponent::GetBondInsideTime() const
{
	return BondInsideTime;
}

void UEnemySpawnerComponent::HandleBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (!IsBondActor(OtherActor))
	{
		return;
	}

	CurrentBondActor = OtherActor;
}

void UEnemySpawnerComponent::HandleEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex
)
{
	if (OtherActor != CurrentBondActor)
	{
		return;
	}

	CurrentBondActor = nullptr;

	if (bResetInsideTimeOnBondExit)
	{
		BondInsideTime = 0.0f;
	}
}

bool UEnemySpawnerComponent::IsBondActor(const AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	if (Actor->IsA<AJamesBondCharacter>())
	{
		return true;
	}

	const UWorld* World = GetWorld();
	return World && Actor == UGameplayStatics::GetPlayerPawn(World, 0);
}

bool UEnemySpawnerComponent::CanActivate() const
{
	return
		SWATClass &&
		ActivationCount < MaxActivationCount &&
		SpawnCountPerActivation > 0;
}

void UEnemySpawnerComponent::ActivateSpawner()
{
	if (!CurrentBondActor || !CanActivate())
	{
		return;
	}

	for (int32 SpawnIndex = 0; SpawnIndex < SpawnCountPerActivation; ++SpawnIndex)
	{
		SpawnSWAT(CurrentBondActor);
	}

	++ActivationCount;
}

void UEnemySpawnerComponent::SpawnSWAT(AActor* BondActor)
{
	UWorld* World = GetWorld();

	if (!World || !SWATClass)
	{
		return;
	}

	const FTransform SpawnTransform = GetSpawnTransform(NextSpawnPointIndex);
	++NextSpawnPointIndex;

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ASWATEnemyCharacter* SpawnedSWAT = World->SpawnActor<ASWATEnemyCharacter>(
		SWATClass,
		SpawnTransform,
		SpawnParameters
	);

	if (!SpawnedSWAT)
	{
		return;
	}

	SpawnedSWAT->SpawnDefaultController();
	AssignTargetToSpawnedSWAT(SpawnedSWAT, BondActor);
}

FTransform UEnemySpawnerComponent::GetSpawnTransform(int32 SpawnIndex) const
{
	FTransform SpawnTransform = GetComponentTransform();
	const int32 SpawnPointCount = SpawnPoints.Num();

	if (SpawnPointCount > 0)
	{
		const int32 WrappedIndex = SpawnIndex % SpawnPointCount;
		const ATargetPoint* SpawnPoint = SpawnPoints[WrappedIndex];

		if (SpawnPoint)
		{
			SpawnTransform = SpawnPoint->GetActorTransform();
		}
	}
	else
	{
		const int32 ManualSpawnCount = ManualSpawnTransforms.Num();

		if (ManualSpawnCount > 0)
		{
			const int32 WrappedIndex = SpawnIndex % ManualSpawnCount;
			SpawnTransform = ManualSpawnTransforms[WrappedIndex] * GetComponentTransform();
		}
	}

	FVector SpawnLocation = SpawnTransform.GetLocation();

	if (SpawnHorizontalJitterRadius > 0.0f)
	{
		const FVector2D RandomOffset =
			FMath::RandPointInCircle(SpawnHorizontalJitterRadius);
		SpawnLocation.X += RandomOffset.X;
		SpawnLocation.Y += RandomOffset.Y;
	}

	SpawnLocation.Z += SpawnZOffset;
	SpawnTransform.SetLocation(SpawnLocation);

	return SpawnTransform;
}

void UEnemySpawnerComponent::AssignTargetToSpawnedSWAT(
	ASWATEnemyCharacter* SpawnedSWAT,
	AActor* BondActor
) const
{
	if (!SpawnedSWAT || !BondActor)
	{
		return;
	}

	ASWATAIController* SWATAIController =
		Cast<ASWATAIController>(SpawnedSWAT->GetController());

	if (!SWATAIController)
	{
		return;
	}

	SWATAIController->SetTargetActor(BondActor);
}
