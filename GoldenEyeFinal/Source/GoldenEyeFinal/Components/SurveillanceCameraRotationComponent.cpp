#include "SurveillanceCameraRotationComponent.h"

#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"

USurveillanceCameraRotationComponent::USurveillanceCameraRotationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USurveillanceCameraRotationComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheTargetComponent();
	CaptureBaseRotation();

	CurrentYawOffset = GetLeftLimit();
	Direction = 1.0f;
	ApplyYawOffset(CurrentYawOffset);

	if (bStartScanningOnBeginPlay)
	{
		StartScanning();
	}
}

void USurveillanceCameraRotationComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsScanning || DegreesPerSecond <= 0.0f)
	{
		return;
	}

	if (!CachedTargetComponent)
	{
		CacheTargetComponent();
		CaptureBaseRotation();
	}

	if (!CachedTargetComponent)
	{
		return;
	}

	if (PauseTimer > 0.0f)
	{
		PauseTimer = FMath::Max(0.0f, PauseTimer - DeltaTime);
		return;
	}

	const float LeftLimit = GetLeftLimit();
	const float RightLimit = GetRightLimit();

	CurrentYawOffset += Direction * DegreesPerSecond * DeltaTime;

	if (CurrentYawOffset >= RightLimit)
	{
		CurrentYawOffset = RightLimit;
		Direction = -1.0f;
		PauseTimer = PauseAtEndsSeconds;
	}
	else if (CurrentYawOffset <= LeftLimit)
	{
		CurrentYawOffset = LeftLimit;
		Direction = 1.0f;
		PauseTimer = PauseAtEndsSeconds;
	}

	ApplyYawOffset(CurrentYawOffset);
}

void USurveillanceCameraRotationComponent::StartScanning()
{
	bIsScanning = true;
}

void USurveillanceCameraRotationComponent::StopScanning()
{
	bIsScanning = false;
}

void USurveillanceCameraRotationComponent::SetTargetComponentByName(FName ComponentName)
{
	TargetComponentName = ComponentName;
	TargetComponent = FComponentReference();
	CacheTargetComponent();
	CaptureBaseRotation();
	ApplyYawOffset(CurrentYawOffset);
}

void USurveillanceCameraRotationComponent::ResetToCenter()
{
	CurrentYawOffset = 0.0f;
	PauseTimer = 0.0f;
	ApplyYawOffset(CurrentYawOffset);
}

bool USurveillanceCameraRotationComponent::IsScanning() const
{
	return bIsScanning;
}

USceneComponent* USurveillanceCameraRotationComponent::GetTargetSceneComponent() const
{
	return CachedTargetComponent ? CachedTargetComponent.Get() : ResolveTargetComponent();
}

USceneComponent* USurveillanceCameraRotationComponent::ResolveTargetComponent() const
{
	AActor* OwnerActor = GetOwner();

	if (!OwnerActor)
	{
		return nullptr;
	}

	if (USceneComponent* PickedComponent =
		Cast<USceneComponent>(TargetComponent.GetComponent(OwnerActor)))
	{
		return PickedComponent;
	}

	if (!TargetComponentName.IsNone())
	{
		TArray<USceneComponent*> SceneComponents;
		OwnerActor->GetComponents<USceneComponent>(SceneComponents);

		for (USceneComponent* SceneComponent : SceneComponents)
		{
			if (SceneComponent && SceneComponent->GetFName() == TargetComponentName)
			{
				return SceneComponent;
			}
		}
	}

	return bUseOwnerRootWhenTargetMissing ? OwnerActor->GetRootComponent() : nullptr;
}

void USurveillanceCameraRotationComponent::CacheTargetComponent()
{
	CachedTargetComponent = ResolveTargetComponent();
}

void USurveillanceCameraRotationComponent::CaptureBaseRotation()
{
	if (!CachedTargetComponent)
	{
		return;
	}

	BaseRelativeRotation = CachedTargetComponent->GetRelativeRotation();
	BaseWorldRotation = CachedTargetComponent->GetComponentRotation();
}

void USurveillanceCameraRotationComponent::ApplyYawOffset(float YawOffset)
{
	if (!CachedTargetComponent)
	{
		return;
	}

	const FRotator TargetRotation =
		bUseLocalRotation
			? BaseRelativeRotation + FRotator(0.0f, YawOffset, 0.0f)
			: BaseWorldRotation + FRotator(0.0f, YawOffset, 0.0f);

	if (bUseLocalRotation)
	{
		CachedTargetComponent->SetRelativeRotation(TargetRotation);
		return;
	}

	CachedTargetComponent->SetWorldRotation(TargetRotation);
}

float USurveillanceCameraRotationComponent::GetLeftLimit() const
{
	return -FMath::Abs(LeftYawDegrees);
}

float USurveillanceCameraRotationComponent::GetRightLimit() const
{
	return FMath::Abs(RightYawDegrees);
}
