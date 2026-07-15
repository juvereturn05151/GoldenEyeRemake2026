#include "BondTimeSlowComponent.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UBondTimeSlowComponent::UBondTimeSlowComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBondTimeSlowComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentMeter = MaxMeter;

	const UWorld* World = GetWorld();
	LastRealTimeSeconds = World ? World->GetRealTimeSeconds() : 0.0f;

	BroadcastMeterChanged();
}

void UBondTimeSlowComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearTimeDilation();

	Super::EndPlay(EndPlayReason);
}

void UBondTimeSlowComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	const float CurrentRealTimeSeconds =
		World->GetRealTimeSeconds();

	const float RealDeltaSeconds =
		LastRealTimeSeconds > 0.0f
			? CurrentRealTimeSeconds - LastRealTimeSeconds
			: DeltaTime;

	LastRealTimeSeconds = CurrentRealTimeSeconds;

	if (bIsActive)
	{
		DrainMeter(RealDeltaSeconds);
		return;
	}

	InactiveRealSeconds += RealDeltaSeconds;

	if (InactiveRealSeconds >= RegenerationDelay)
	{
		RegenerateMeter(RealDeltaSeconds);
	}
}

void UBondTimeSlowComponent::StartTimeSlow()
{
	if (!CanStartTimeSlow())
	{
		return;
	}

	bIsActive = true;
	InactiveRealSeconds = 0.0f;

	ApplyTimeDilation();
	OnTimeSlowStateChanged.Broadcast(true);
}

void UBondTimeSlowComponent::StopTimeSlow()
{
	if (!bIsActive)
	{
		return;
	}

	bIsActive = false;
	InactiveRealSeconds = 0.0f;

	ClearTimeDilation();
	OnTimeSlowStateChanged.Broadcast(false);
}

bool UBondTimeSlowComponent::CanStartTimeSlow() const
{
	return !bIsActive && CurrentMeter > 0.0f;
}

bool UBondTimeSlowComponent::IsTimeSlowActive() const
{
	return bIsActive;
}

float UBondTimeSlowComponent::GetCurrentMeter() const
{
	return CurrentMeter;
}

float UBondTimeSlowComponent::GetMaxMeter() const
{
	return MaxMeter;
}

float UBondTimeSlowComponent::GetMeterPercent() const
{
	return MaxMeter > 0.0f ? CurrentMeter / MaxMeter : 0.0f;
}

void UBondTimeSlowComponent::DrainMeter(float RealDeltaSeconds)
{
	if (DrainPerRealSecond <= 0.0f)
	{
		return;
	}

	SetCurrentMeter(
		CurrentMeter - (DrainPerRealSecond * RealDeltaSeconds)
	);

	if (CurrentMeter <= 0.0f)
	{
		StopTimeSlow();
	}
}

void UBondTimeSlowComponent::RegenerateMeter(float RealDeltaSeconds)
{
	if (
		RegenerationPerRealSecond <= 0.0f ||
		CurrentMeter >= MaxMeter
		)
	{
		return;
	}

	SetCurrentMeter(
		CurrentMeter + (RegenerationPerRealSecond * RealDeltaSeconds)
	);
}

void UBondTimeSlowComponent::SetCurrentMeter(float NewMeter)
{
	const float ClampedMeter =
		FMath::Clamp(NewMeter, 0.0f, MaxMeter);

	if (FMath::IsNearlyEqual(CurrentMeter, ClampedMeter))
	{
		return;
	}

	CurrentMeter = ClampedMeter;
	BroadcastMeterChanged();
}

void UBondTimeSlowComponent::ApplyTimeDilation()
{
	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	UGameplayStatics::SetGlobalTimeDilation(
		World,
		TimeDilation
	);

	AActor* OwnerActor = GetOwner();

	if (OwnerActor)
	{
		OwnerActor->CustomTimeDilation =
			TimeDilation > 0.0f ? 1.0f / TimeDilation : 1.0f;
	}
}

void UBondTimeSlowComponent::ClearTimeDilation()
{
	UWorld* World = GetWorld();

	if (World)
	{
		UGameplayStatics::SetGlobalTimeDilation(World, 1.0f);
	}

	AActor* OwnerActor = GetOwner();

	if (OwnerActor)
	{
		OwnerActor->CustomTimeDilation = 1.0f;
	}
}

void UBondTimeSlowComponent::BroadcastMeterChanged()
{
	OnMeterChanged.Broadcast(
		CurrentMeter,
		MaxMeter
	);
}
