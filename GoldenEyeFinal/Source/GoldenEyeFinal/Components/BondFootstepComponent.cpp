#include "BondFootstepComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UBondFootstepComponent::UBondFootstepComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBondFootstepComponent::BeginPlay()
{
	Super::BeginPlay();

	StepTimer = 0.0f;
}

void UBondFootstepComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	float Speed = 0.0f;

	if (!ShouldPlayFootsteps(Speed))
	{
		StepTimer = 0.0f;
		return;
	}

	StepTimer -= DeltaTime;

	if (StepTimer > 0.0f)
	{
		return;
	}

	PlayFootstep();
	StepTimer = GetCurrentStepInterval(Speed);
}

bool UBondFootstepComponent::ShouldPlayFootsteps(float& OutSpeed) const
{
	OutSpeed = 0.0f;

	const ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner());

	if (!CharacterOwner || !FootstepSound)
	{
		return false;
	}

	const UCharacterMovementComponent* MovementComponent =
		CharacterOwner->GetCharacterMovement();

	if (!MovementComponent || !MovementComponent->IsMovingOnGround())
	{
		return false;
	}

	const FVector Velocity = CharacterOwner->GetVelocity();
	OutSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();

	return OutSpeed >= MinimumSpeed;
}

float UBondFootstepComponent::GetCurrentStepInterval(float Speed) const
{
	return Speed >= RunSpeedThreshold ? RunStepInterval : WalkStepInterval;
}

void UBondFootstepComponent::PlayFootstep()
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();

	if (!OwnerActor || !World || !FootstepSound)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(
		World,
		FootstepSound,
		OwnerActor->GetActorLocation(),
		FootstepVolume,
		FootstepPitch
	);
}
