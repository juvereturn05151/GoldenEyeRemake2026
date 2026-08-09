#include "JamesBondCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "../Components/BondFootstepComponent.h"
#include "../Components/BondHealthComponent.h"
#include "../Components/BondTimeSlowComponent.h"
#include "../Components/BondWeaponComponent.h"
#include "../Weapons/BondWeaponBase.h"

AJamesBondCharacter::AJamesBondCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;

	// ACharacter provides CharacterMesh0, but Bond is first-person arms only.
	GetMesh()->SetHiddenInGame(true);
	GetMesh()->SetVisibility(false);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FirstPersonCamera =CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));

	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());

	FirstPersonCamera->SetRelativeLocation(FVector(-10.0f, 0.0f, 64.0f));

	FirstPersonCamera->bUsePawnControlRotation = true;

	FirstPersonArms =CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonArms"));

	FirstPersonArms->SetupAttachment(FirstPersonCamera);

	FirstPersonArms->SetOnlyOwnerSee(true);

	FirstPersonArms->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FirstPersonArms->SetGenerateOverlapEvents(false);
	FirstPersonArms->SetCastShadow(false);

	WeaponRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponRoot"));

	WeaponRoot->SetupAttachment(FirstPersonArms);

	BondAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BondAudioComponent"));

	BondAudioComponent->SetupAttachment(GetRootComponent());

	BondAudioComponent->bAutoActivate = false;

	HealthComponent =CreateDefaultSubobject<UBondHealthComponent>(TEXT("HealthComponent"));

	WeaponComponent =CreateDefaultSubobject<UBondWeaponComponent>(TEXT("WeaponComponent"));

	TimeSlowComponent =CreateDefaultSubobject<UBondTimeSlowComponent>(TEXT("TimeSlowComponent"));

	FootstepComponent = CreateDefaultSubobject<UBondFootstepComponent>(TEXT("FootstepComponent"));
}

void AJamesBondCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddUniqueDynamic(
			this,
			&AJamesBondCharacter::HandleDeath
		);
	}

	AttachWeaponRootToConfiguredSocket();
	InitializeInputMapping();
}

void AJamesBondCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDeathFalling)
	{
		UpdateDeathFall(DeltaTime);
	}
}

void AJamesBondCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput =Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (!EnhancedInput)
	{
		UE_LOG(LogTemp,Error,TEXT("JamesBondCharacter requires Enhanced Input."));
		return;
	}

	if (MoveAction)
	{
		EnhancedInput->BindAction(
			MoveAction,
			ETriggerEvent::Triggered,
			this,
			&AJamesBondCharacter::Move
		);
	}

	if (LookAction)
	{
		EnhancedInput->BindAction(
			LookAction,
			ETriggerEvent::Triggered,
			this,
			&AJamesBondCharacter::Look
		);
	}

	if (JumpAction)
	{
		EnhancedInput->BindAction(
			JumpAction,
			ETriggerEvent::Started,
			this,
			&AJamesBondCharacter::StartJump
		);

		EnhancedInput->BindAction(
			JumpAction,
			ETriggerEvent::Completed,
			this,
			&AJamesBondCharacter::StopJump
		);

		EnhancedInput->BindAction(
			JumpAction,
			ETriggerEvent::Canceled,
			this,
			&AJamesBondCharacter::StopJump
		);
	}

	if (FireAction)
	{
		EnhancedInput->BindAction(
			FireAction,
			ETriggerEvent::Started,
			this,
			&AJamesBondCharacter::HandleFireStarted
		);

		EnhancedInput->BindAction(
			FireAction,
			ETriggerEvent::Completed,
			this,
			&AJamesBondCharacter::HandleFireCompleted
		);

		EnhancedInput->BindAction(
			FireAction,
			ETriggerEvent::Canceled,
			this,
			&AJamesBondCharacter::HandleFireCompleted
		);
	}

	if(ReloadAction)
	{
		EnhancedInput->BindAction(
			ReloadAction,
			ETriggerEvent::Started,
			this,
			&AJamesBondCharacter::HandleReload
		);
	}

	if (TimeSlowAction)
	{
		EnhancedInput->BindAction(
			TimeSlowAction,
			ETriggerEvent::Started,
			this,
			&AJamesBondCharacter::HandleTimeSlowStarted
		);

		EnhancedInput->BindAction(
			TimeSlowAction,
			ETriggerEvent::Completed,
			this,
			&AJamesBondCharacter::HandleTimeSlowCompleted
		);

		EnhancedInput->BindAction(
			TimeSlowAction,
			ETriggerEvent::Canceled,
			this,
			&AJamesBondCharacter::HandleTimeSlowCompleted
		);
	}
}

void AJamesBondCharacter::Move(const FInputActionValue& Value)
{
	if (bIsDeathFalling)
	{
		return;
	}

	if (!Controller)
	{
		return;
	}

	const FVector2D Input = Value.Get<FVector2D>();

	const FRotator ControlRotation = Controller->GetControlRotation();

	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, Input.Y);
	AddMovementInput(Right, Input.X);
}

void AJamesBondCharacter::Look(const FInputActionValue& Value)
{
	if (bIsDeathFalling)
	{
		return;
	}

	const FVector2D Input = Value.Get<FVector2D>();

	AddControllerYawInput(Input.X);
	AddControllerPitchInput(Input.Y);
}

void AJamesBondCharacter::StartJump()
{
	if (bIsDeathFalling)
	{
		return;
	}

	Jump();
}

void AJamesBondCharacter::StopJump()
{
	StopJumping();
}

void AJamesBondCharacter::HandleFireStarted()
{
	if (bIsDeathFalling)
	{
		return;
	}

	if (WeaponComponent)
	{
		WeaponComponent->StartFire();
	}
}

void AJamesBondCharacter::HandleFireCompleted()
{
	if (WeaponComponent)
	{
		WeaponComponent->StopFire();
	}
}

void AJamesBondCharacter::HandleReload()
{
	if (bIsDeathFalling)
	{
		return;
	}

	if (WeaponComponent)
	{
		WeaponComponent->Reload();
	}
}

void AJamesBondCharacter::CompleteReload()
{
	if (!WeaponComponent)
	{
		return;
	}

	ABondWeaponBase* EquippedWeapon = WeaponComponent->GetEquippedWeapon();

	if (EquippedWeapon)
	{
		EquippedWeapon->CompleteReload();
	}
}

void AJamesBondCharacter::HandleTimeSlowStarted()
{
	if (bIsDeathFalling)
	{
		return;
	}

	if (TimeSlowComponent)
	{
		TimeSlowComponent->StartTimeSlow();
	}
}

void AJamesBondCharacter::HandleTimeSlowCompleted()
{
	if (TimeSlowComponent)
	{
		TimeSlowComponent->StopTimeSlow();
	}
}

void AJamesBondCharacter::AttachWeaponRootToConfiguredSocket()
{
	if (WeaponRootSocketName == NAME_None)
	{
		return;
	}

	if (!FirstPersonArms || !WeaponRoot)
	{
		return;
	}

	if (!FirstPersonArms->DoesSocketExist(WeaponRootSocketName))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("%s could not attach WeaponRoot: socket '%s' does not exist on FirstPersonArms."),
			*GetName(),
			*WeaponRootSocketName.ToString()
		);
		return;
	}

	WeaponRoot->AttachToComponent(
		FirstPersonArms,
		FAttachmentTransformRules::KeepWorldTransform,
		WeaponRootSocketName
	);
}

void AJamesBondCharacter::InitializeInputMapping()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());

	if (!PC || !PC->GetLocalPlayer())
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem =PC->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

	if (Subsystem && DefaultMappingContext)
	{
		Subsystem->AddMappingContext(DefaultMappingContext,0);
	}
}

void AJamesBondCharacter::HandleDeath()
{
	StartDeathFall();
}

void AJamesBondCharacter::StartDeathFall()
{
	if (bIsDeathFalling)
	{
		return;
	}

	DisableBondOnDeath();

	bIsDeathFalling = true;
	DeathFallElapsed = 0.0f;

	if (FirstPersonCamera)
	{
		FirstPersonCamera->bUsePawnControlRotation = false;
		DeathCameraStartLocation = FirstPersonCamera->GetRelativeLocation();
		DeathCameraStartRotation = FirstPersonCamera->GetRelativeRotation();
		DeathCameraTargetLocation = DeathCameraStartLocation + DeathCameraLocationOffset;
		DeathCameraTargetRotation = DeathCameraStartRotation + DeathCameraRotationOffset;
	}

	SetActorTickEnabled(true);
}

void AJamesBondCharacter::UpdateDeathFall(float DeltaTime)
{
	if (!FirstPersonCamera)
	{
		bIsDeathFalling = false;
		SetActorTickEnabled(false);
		return;
	}

	DeathFallElapsed += DeltaTime;

	const float Alpha = FMath::Clamp(
		DeathFallElapsed / FMath::Max(DeathFallDuration, KINDA_SMALL_NUMBER),
		0.0f,
		1.0f
	);

	const float SmoothedAlpha = FMath::InterpEaseOut(0.0f, 1.0f, Alpha, 2.0f);

	FirstPersonCamera->SetRelativeLocation(
		FMath::Lerp(DeathCameraStartLocation, DeathCameraTargetLocation, SmoothedAlpha)
	);

	FirstPersonCamera->SetRelativeRotation(
		FMath::Lerp(DeathCameraStartRotation, DeathCameraTargetRotation, SmoothedAlpha)
	);

	if (Alpha >= 1.0f)
	{
		bIsDeathFalling = false;
		SetActorTickEnabled(false);
	}
}

void AJamesBondCharacter::DisableBondOnDeath()
{
	if (WeaponComponent)
	{
		WeaponComponent->StopFire();
	}

	if (TimeSlowComponent)
	{
		TimeSlowComponent->StopTimeSlow();
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		PlayerController->SetIgnoreMoveInput(true);
		PlayerController->SetIgnoreLookInput(true);
	}
}

USceneComponent* AJamesBondCharacter::GetWeaponRoot() const
{
	return WeaponRoot;
}

USkeletalMeshComponent* AJamesBondCharacter::GetFirstPersonArms() const
{
	return FirstPersonArms;
}

UBondHealthComponent* AJamesBondCharacter::GetHealthComponent() const
{
	return HealthComponent;
}

UBondWeaponComponent* AJamesBondCharacter::GetWeaponComponent() const
{
	return WeaponComponent;
}

UBondTimeSlowComponent* AJamesBondCharacter::GetTimeSlowComponent() const
{
	return TimeSlowComponent;
}

UBondFootstepComponent* AJamesBondCharacter::GetFootstepComponent() const
{
	return FootstepComponent;
}
