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
#include "../Components/BondHealthComponent.h"
#include "../Components/BondWeaponComponent.h"

AJamesBondCharacter::AJamesBondCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

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
}

void AJamesBondCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitializeInputMapping();
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
}

void AJamesBondCharacter::Move(const FInputActionValue& Value)
{
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
	const FVector2D Input = Value.Get<FVector2D>();

	AddControllerYawInput(Input.X);
	AddControllerPitchInput(Input.Y);
}

void AJamesBondCharacter::StartJump()
{
	Jump();
}

void AJamesBondCharacter::StopJump()
{
	StopJumping();
}

void AJamesBondCharacter::HandleFireStarted()
{
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
	if (WeaponComponent)
	{
		WeaponComponent->Reload();
	}
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

USceneComponent* AJamesBondCharacter::GetWeaponRoot() const
{
	return WeaponRoot;
}

USkeletalMeshComponent* AJamesBondCharacter::GetFirstPersonArms() const
{
	return FirstPersonArms;
}
