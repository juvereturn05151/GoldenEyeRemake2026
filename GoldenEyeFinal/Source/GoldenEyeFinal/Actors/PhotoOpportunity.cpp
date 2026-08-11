#include "PhotoOpportunity.h"

#include "../Characters/JamesBondCharacter.h"
#include "../Mission/GameplayMissionSubsystem.h"
#include "../Mission/MissionTypes.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Brush.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"

APhotoOpportunity::APhotoOpportunity()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	SetRootComponent(TriggerBox);

	TriggerBox->SetBoxExtent(FVector(120.0f, 120.0f, 100.0f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
}

void APhotoOpportunity::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(
			this,
			&APhotoOpportunity::HandleTriggerBeginOverlap
		);

		TriggerBox->OnComponentEndOverlap.AddDynamic(
			this,
			&APhotoOpportunity::HandleTriggerEndOverlap
		);
	}
}

void APhotoOpportunity::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdatePhotoAvailability();
}

void APhotoOpportunity::TryTakePhoto()
{
	if (bPhotoTaken)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s photo attempt failed: photo has already been taken."), *GetName());
		return;
	}

	if (!IsPhotoValid(true))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("%s photo attempt failed: Bond must be inside, facing the target, and have line of sight."),
			*GetName()
		);
		return;
	}

	bPhotoTaken = true;
	SetActorTickEnabled(false);
	SetPhotoAvailable(false);

	UE_LOG(LogTemp, Log, TEXT("%s photo taken successfully."), *GetName());
	PlayPhotoTakenSound();
	OnPhotoTaken.Broadcast();
	BroadcastPhotoMissionEvent();
}

void APhotoOpportunity::HandleTriggerBeginOverlap(
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
	Bond->SetPhotoOpportunity(this);

	UE_LOG(LogTemp, Log, TEXT("Enter photo zone: %s."), *GetName());

	if (!bPhotoTaken)
	{
		SetActorTickEnabled(true);
		UpdatePhotoAvailability();
	}
}

void APhotoOpportunity::HandleTriggerEndOverlap(
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

	UE_LOG(LogTemp, Log, TEXT("Leave photo zone: %s."), *GetName());

	bPlayerInside = false;
	SetActorTickEnabled(false);
	Bond->ClearPhotoOpportunity(this);
	SetPhotoAvailable(false);
	OverlappingBond = nullptr;
}

void APhotoOpportunity::UpdatePhotoAvailability()
{
	SetPhotoAvailable(IsPhotoValid());
}

bool APhotoOpportunity::IsPhotoValid(bool bLogFailure) const
{
	if (!bPlayerInside)
	{
		if (bLogFailure)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s photo attempt failed: Bond is not inside the photo zone."), *GetName());
		}

		return false;
	}

	if (bPhotoTaken)
	{
		if (bLogFailure)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s photo attempt failed: this photo was already taken."), *GetName());
		}

		return false;
	}

	if (!OverlappingBond)
	{
		if (bLogFailure)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s photo attempt failed: no overlapping Bond reference was available."), *GetName());
		}

		return false;
	}

	if (!PhotoTarget)
	{
		if (bLogFailure)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s photo attempt failed: PhotoTarget is not assigned."), *GetName());
		}

		return false;
	}

	FVector CameraLocation = FVector::ZeroVector;
	FVector CameraForward = FVector::ForwardVector;

	if (!GetCameraView(CameraLocation, CameraForward))
	{
		if (bLogFailure)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s photo attempt failed: could not read Bond camera viewpoint."), *GetName());
		}

		return false;
	}

	return IsFacingTarget(CameraLocation, CameraForward, bLogFailure)
		&& IsTargetVisible(CameraLocation, bLogFailure);
}

bool APhotoOpportunity::GetCameraView(FVector& OutCameraLocation, FVector& OutCameraForward) const
{
	if (!OverlappingBond)
	{
		return false;
	}

	AController* Controller = OverlappingBond->GetController();

	if (!Controller)
	{
		return false;
	}

	FRotator CameraRotation = FRotator::ZeroRotator;
	Controller->GetPlayerViewPoint(OutCameraLocation, CameraRotation);
	OutCameraForward = CameraRotation.Vector();

	return true;
}

FVector APhotoOpportunity::GetPhotoTargetViewLocation() const
{
	if (!PhotoTarget)
	{
		return FVector::ZeroVector;
	}

	FVector Origin = FVector::ZeroVector;
	FVector BoxExtent = FVector::ZeroVector;
	PhotoTarget->GetActorBounds(true, Origin, BoxExtent);

	if (BoxExtent.IsNearlyZero())
	{
		return PhotoTarget->GetActorLocation();
	}

	return Origin;
}

bool APhotoOpportunity::IsFacingTarget(
	const FVector& CameraLocation,
	const FVector& CameraForward,
	bool bLogFailure
) const
{
	if (!PhotoTarget)
	{
		return false;
	}

	const FVector TargetViewLocation = GetPhotoTargetViewLocation();
	const FVector DirectionToTarget = (TargetViewLocation - CameraLocation).GetSafeNormal();

	if (DirectionToTarget.IsNearlyZero())
	{
		if (bLogFailure)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s photo attempt failed: camera is too close to PhotoTarget view location."), *GetName());
		}

		return false;
	}

	const float FacingDot = FVector::DotProduct(CameraForward.GetSafeNormal(), DirectionToTarget);

	if (FacingDot < FacingDotThreshold && bLogFailure)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("%s photo attempt failed: facing dot %.2f is below threshold %.2f."),
			*GetName(),
			FacingDot,
			FacingDotThreshold
		);
	}

	return FacingDot >= FacingDotThreshold;
}

bool APhotoOpportunity::IsTargetVisible(const FVector& CameraLocation, bool bLogFailure) const
{
	if (!PhotoTarget || !OverlappingBond)
	{
		return false;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		return false;
	}

	FHitResult Hit;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PhotoOpportunityVisibility), false);
	QueryParams.AddIgnoredActor(OverlappingBond);
	QueryParams.AddIgnoredActor(this);

	const FVector TraceEnd = GetPhotoTargetViewLocation();

	const bool bHit = World->LineTraceSingleByChannel(
		Hit,
		CameraLocation,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

	if (!bHit)
	{
		return true;
	}

	AActor* HitActor = Hit.GetActor();
	const bool bHitTarget = IsTraceHitOnPhotoTarget(Hit);

	if (!bHitTarget && bLogFailure)
	{
		UPrimitiveComponent* HitComponent = Hit.GetComponent();

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("%s photo attempt failed: visibility trace hit %s component %s before PhotoTarget %s."),
			*GetName(),
			HitActor ? *HitActor->GetName() : TEXT("no actor"),
			HitComponent ? *HitComponent->GetName() : TEXT("no component"),
			*PhotoTarget->GetName()
		);
	}

	return bHitTarget;
}

bool APhotoOpportunity::IsTraceHitOnPhotoTarget(const FHitResult& Hit) const
{
	if (!PhotoTarget)
	{
		return false;
	}

	AActor* HitActor = Hit.GetActor();

	if (HitActor == PhotoTarget || (HitActor && HitActor->IsAttachedTo(PhotoTarget)))
	{
		return true;
	}

	if (UPrimitiveComponent* HitComponent = Hit.GetComponent())
	{
		if (HitComponent->GetOwner() == PhotoTarget)
		{
			return true;
		}

		if (!HitActor && PhotoTarget->IsA<ABrush>() && HitComponent->GetName().StartsWith(TEXT("ModelComponent")))
		{
			return true;
		}
	}

	FVector TargetOrigin = FVector::ZeroVector;
	FVector TargetExtent = FVector::ZeroVector;
	PhotoTarget->GetActorBounds(true, TargetOrigin, TargetExtent);

	if (TargetExtent.IsNearlyZero())
	{
		return false;
	}

	const FBox TargetBounds(
		TargetOrigin - TargetExtent,
		TargetOrigin + TargetExtent
	);

	const FVector BoundsTolerance(25.0f, 25.0f, 25.0f);

	return TargetBounds.ExpandBy(BoundsTolerance).IsInsideOrOn(Hit.ImpactPoint);
}

void APhotoOpportunity::SetPhotoAvailable(bool bNewAvailable)
{
	if (bPhotoAvailable == bNewAvailable)
	{
		return;
	}

	bPhotoAvailable = bNewAvailable;

	if (bPhotoAvailable)
	{
		UE_LOG(LogTemp, Log, TEXT("%s photo available."), *GetName());
		OnPhotoAvailable.Broadcast();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("%s photo unavailable."), *GetName());
		OnPhotoUnavailable.Broadcast();
	}
}

void APhotoOpportunity::PlayPhotoTakenSound() const
{
	if (!PhotoTakenSound)
	{
		return;
	}

	UGameplayStatics::PlaySound2D(
		this,
		PhotoTakenSound,
		PhotoTakenSoundVolume,
		PhotoTakenSoundPitch
	);
}

void APhotoOpportunity::BroadcastPhotoMissionEvent()
{
	if (!bBroadcastMissionEventOnPhotoTaken)
	{
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission] Photo event failed: missing world PhotoOpportunity=%s"), *GetName());
		return;
	}

	UGameplayMissionSubsystem* MissionSubsystem = World->GetSubsystem<UGameplayMissionSubsystem>();

	if (!MissionSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission] Photo event failed: missing mission subsystem PhotoOpportunity=%s"), *GetName());
		return;
	}

	FMissionEventData EventData;
	EventData.EventTag = MissionEventTag;
	EventData.Instigator = OverlappingBond;
	EventData.Target = PhotoTarget ? PhotoTarget : this;
	EventData.ContextId = MissionContextId;
	EventData.Amount = 1;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[Mission] Photo taken event PhotoOpportunity=%s Event=%s Context=%s Target=%s"),
		*GetName(),
		*MissionEventTag.ToString(),
		*MissionContextId.ToString(),
		*GetNameSafe(EventData.Target)
	);

	MissionSubsystem->BroadcastMissionEvent(EventData);
}
