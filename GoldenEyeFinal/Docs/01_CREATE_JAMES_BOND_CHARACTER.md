# Step 1 — Create `AJamesBondCharacter`

This stage creates:

- Movement
- Mouse look
- Jump
- First-person camera
- First-person arms
- Weapon attachment point
- Character audio component

Do not add health, weapons, shooting, reload, UI, time slow, or interaction yet.

## 1. Create the C++ class

In Unreal Editor:

1. Open **Tools → New C++ Class**.
2. Choose **Character**.
3. Name it `JamesBondCharacter`.
4. Create the class.
5. Close Unreal Editor before compiling from Visual Studio.

Expected files:

```text
JamesBondCharacter.h
JamesBondCharacter.cpp
```

## 2. Target component hierarchy

```text
CapsuleComponent
├── Mesh (CharacterMesh0)      ← leave empty and hidden
├── FirstPersonCamera
│   └── FirstPersonArms
│       └── WeaponRoot
├── BondAudioComponent
└── CharacterMovement
```

Do not add:

```text
BodyMesh
ThirdPersonMesh
Permanent Gun component
Permanent SpotLight
```

## 3. `JamesBondCharacter.h`

```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "JamesBondCharacter.generated.h"

class UAudioComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class USceneComponent;
class USkeletalMeshComponent;

UCLASS()
class GOLDENEYEFINAL_API AJamesBondCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AJamesBondCharacter();

	UFUNCTION(BlueprintPure, Category = "Bond|Components")
	USceneComponent* GetWeaponRoot() const;

	UFUNCTION(BlueprintPure, Category = "Bond|Components")
	USkeletalMeshComponent* GetFirstPersonArms() const;

protected:
	virtual void BeginPlay() override;

	virtual void SetupPlayerInputComponent(
		UInputComponent* PlayerInputComponent
	) override;

private:
	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Bond|Components",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Bond|Components",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<USkeletalMeshComponent> FirstPersonArms;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Bond|Components",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<USceneComponent> WeaponRoot;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Bond|Components",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<UAudioComponent> BondAudioComponent;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Bond|Input",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Bond|Input",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Bond|Input",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Bond|Input",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<UInputAction> JumpAction;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartJump();
	void StopJump();
	void InitializeInputMapping();
};
```

Important:

```cpp
#include "JamesBondCharacter.generated.h"
```

must remain the final include in the header.

## 4. `JamesBondCharacter.cpp`

```cpp
#include "JamesBondCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

AJamesBondCharacter::AJamesBondCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;

	// ACharacter always includes an inherited mesh component.
	// This project does not use it.
	GetMesh()->SetHiddenInGame(true);
	GetMesh()->SetVisibility(false);
	GetMesh()->SetCollisionEnabled(
		ECollisionEnabled::NoCollision
	);

	FirstPersonCamera =
		CreateDefaultSubobject<UCameraComponent>(
			TEXT("FirstPersonCamera")
		);

	FirstPersonCamera->SetupAttachment(
		GetCapsuleComponent()
	);

	FirstPersonCamera->SetRelativeLocation(
		FVector(-10.0f, 0.0f, 64.0f)
	);

	FirstPersonCamera->bUsePawnControlRotation = true;

	FirstPersonArms =
		CreateDefaultSubobject<USkeletalMeshComponent>(
			TEXT("FirstPersonArms")
		);

	FirstPersonArms->SetupAttachment(
		FirstPersonCamera
	);

	FirstPersonArms->SetOnlyOwnerSee(true);

	FirstPersonArms->SetCollisionEnabled(
		ECollisionEnabled::NoCollision
	);

	FirstPersonArms->SetGenerateOverlapEvents(false);
	FirstPersonArms->SetCastShadow(false);

	WeaponRoot =
		CreateDefaultSubobject<USceneComponent>(
			TEXT("WeaponRoot")
		);

	WeaponRoot->SetupAttachment(
		FirstPersonArms
	);

	BondAudioComponent =
		CreateDefaultSubobject<UAudioComponent>(
			TEXT("BondAudioComponent")
		);

	BondAudioComponent->SetupAttachment(
		GetRootComponent()
	);

	BondAudioComponent->bAutoActivate = false;
}

void AJamesBondCharacter::BeginPlay()
{
	Super::BeginPlay();
	InitializeInputMapping();
}

void AJamesBondCharacter::SetupPlayerInputComponent(
	UInputComponent* PlayerInputComponent
)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput =
		Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (!EnhancedInput)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("JamesBondCharacter requires Enhanced Input.")
		);
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
}

void AJamesBondCharacter::Move(
	const FInputActionValue& Value
)
{
	if (!Controller)
	{
		return;
	}

	const FVector2D Input =
		Value.Get<FVector2D>();

	const FRotator ControlRotation =
		Controller->GetControlRotation();

	const FRotator YawRotation(
		0.0f,
		ControlRotation.Yaw,
		0.0f
	);

	const FVector Forward =
		FRotationMatrix(YawRotation)
		.GetUnitAxis(EAxis::X);

	const FVector Right =
		FRotationMatrix(YawRotation)
		.GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, Input.Y);
	AddMovementInput(Right, Input.X);
}

void AJamesBondCharacter::Look(
	const FInputActionValue& Value
)
{
	const FVector2D Input =
		Value.Get<FVector2D>();

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

void AJamesBondCharacter::InitializeInputMapping()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	APlayerController* PC =
		Cast<APlayerController>(GetController());

	if (!PC || !PC->GetLocalPlayer())
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		PC->GetLocalPlayer()->GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem
		>();

	if (Subsystem && DefaultMappingContext)
	{
		Subsystem->AddMappingContext(
			DefaultMappingContext,
			0
		);
	}
}

USceneComponent*
AJamesBondCharacter::GetWeaponRoot() const
{
	return WeaponRoot;
}

USkeletalMeshComponent*
AJamesBondCharacter::GetFirstPersonArms() const
{
	return FirstPersonArms;
}
```

## 5. Build.cs

Ensure the project module includes:

```csharp
"EnhancedInput"
```

Example:

```csharp
PublicDependencyModuleNames.AddRange(
	new string[]
	{
		"Core",
		"CoreUObject",
		"Engine",
		"InputCore",
		"EnhancedInput"
	}
);
```

## 6. Compile checkpoint

1. Save all files.
2. Close Unreal Editor.
3. Disable Live Coding.
4. Select `Development Editor`.
5. Select `Win64`.
6. Build `GoldenEyeFinal`.
7. Continue only after the build succeeds.

## 7. Create input assets

Create:

```text
IA_Move
IA_Look
IA_Jump
IMC_JamesBond
```

Settings:

```text
IA_Move = Axis2D
IA_Look = Axis2D
IA_Jump = Digital
```

Movement mapping:

```text
W = Y +1 using Swizzle YXZ
S = Y -1 using Negate + Swizzle YXZ
A = X -1 using Negate
D = X +1
```

Other mappings:

```text
Mouse XY 2D-Axis = IA_Look
Space Bar = IA_Jump
```

## 8. Create `BP_JamesBond`

Create a Blueprint child of `AJamesBondCharacter`.

Name it:

```text
BP_JamesBond
```

Recommended folder:

```text
Content/Blueprints/Characters/Player/
```

## 9. Blueprint setup

Expected hierarchy:

```text
CapsuleComponent
├── Mesh (CharacterMesh0)
├── FirstPersonCamera
│   └── FirstPersonArms
│       └── WeaponRoot
├── BondAudioComponent
└── CharacterMovement
```

Configure `Mesh (CharacterMesh0)`:

```text
Skeletal Mesh = None
Animation Class = None
Hidden in Game = true
Collision Preset = NoCollision
```

Configure `FirstPersonArms`:

```text
Skeletal Mesh = first-person arms mesh
Animation Class = first-person arms Animation Blueprint
Only Owner See = true
Collision Preset = NoCollision
Generate Overlap Events = false
Cast Shadow = false
```

Assign under Class Defaults:

```text
Default Mapping Context = IMC_JamesBond
Move Action = IA_Move
Look Action = IA_Look
Jump Action = IA_Jump
```

## 10. GameMode setup

Create:

```text
BP_GoldenEyeGameMode
```

Set:

```text
Default Pawn Class = BP_JamesBond
```

Assign it in World Settings or Project Settings.

Place one Player Start.

Do not manually place Bond while also using it as the GameMode default pawn.

## Validation

- WASD works.
- Mouse look works.
- Jump works.
- Only one Bond spawns.
- Only one camera exists.
- Only first-person arms are visible.
- The inherited mesh has no assigned mesh.
- Arms have no collision.
- No permanent gun component exists.
- The Blueprint Event Graph is empty.
