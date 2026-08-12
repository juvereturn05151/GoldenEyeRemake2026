#include "BondPlayerController.h"

#include "../Characters/JamesBondCharacter.h"
#include "../Components/BondHealthComponent.h"
#include "../Components/BondTimeSlowComponent.h"
#include "../Components/BondWeaponComponent.h"
#include "../Mission/GameplayMissionSubsystem.h"
#include "../Mission/MissionObjective.h"
#include "../Weapons/BondWeaponBase.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

ABondPlayerController::ABondPlayerController()
{
}

void ABondPlayerController::BeginPlay()
{
	Super::BeginPlay();

	RestoreGameplayInput();
	CreatePlayerWidgets();

	if (MainHUDWidget && TimeSlowHUDWidget)
	{
		OnPlayerWidgetsCreated();
	}

	BindPossessedBondDelegates();
}

void ABondPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	RestoreGameplayInput();
	PossessedBond = Cast<AJamesBondCharacter>(InPawn);
	BindPossessedBondDelegates();
}

UUserWidget* ABondPlayerController::GetMainHUDWidget() const
{
	return MainHUDWidget;
}

UUserWidget* ABondPlayerController::GetTimeSlowHUDWidget() const
{
	return TimeSlowHUDWidget;
}

UUserWidget* ABondPlayerController::GetDeathWidget() const
{
	return DeathWidget;
}

UUserWidget* ABondPlayerController::GetWinWidget() const
{
	return WinWidget;
}

UUserWidget* ABondPlayerController::GetInteractionPromptWidget() const
{
	return InteractionPromptWidget;
}

void ABondPlayerController::ShowInteractionPrompt(const FText& PromptText)
{
	if (!IsLocalController())
	{
		return;
	}

	if (!InteractionPromptWidget && InteractionPromptWidgetClass)
	{
		InteractionPromptWidget = CreateWidget<UUserWidget>(this, InteractionPromptWidgetClass);
	}

	if (!InteractionPromptWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s cannot show interaction prompt: InteractionPromptWidgetClass is not assigned."), *GetName());
		return;
	}

	if (!InteractionPromptWidget->IsInViewport())
	{
		InteractionPromptWidget->AddToViewport(50);
	}

	if (UTextBlock* PromptTextBlock = Cast<UTextBlock>(InteractionPromptWidget->GetWidgetFromName(InteractionPromptTextBlockName)))
	{
		PromptTextBlock->SetText(PromptText);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("%s interaction prompt widget does not contain a TextBlock named '%s'."),
			*GetName(),
			*InteractionPromptTextBlockName.ToString()
		);
	}

	InteractionPromptWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void ABondPlayerController::HideInteractionPrompt()
{
	if (InteractionPromptWidget)
	{
		InteractionPromptWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ABondPlayerController::ShowWinWidget(bool bPauseGame)
{
	if (!IsLocalController())
	{
		return;
	}

	if (!WinWidget && WinWidgetClass)
	{
		WinWidget = CreateWidget<UUserWidget>(this, WinWidgetClass);
	}

	if (!WinWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s cannot show win widget: WinWidgetClass is not assigned."), *GetName());
		return;
	}

	if (!WinWidget->IsInViewport())
	{
		WinWidget->AddToViewport(100);
	}

	WinWidget->SetVisibility(ESlateVisibility::Visible);

	if (bPauseGame)
	{
		SetPause(true);
		bShowMouseCursor = true;

		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(WinWidget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
	}

	HandleBondWin();
}

void ABondPlayerController::InitializeMissionObjectiveUI()
{
	if (!MainHUDWidget)
	{
		CreatePlayerWidgets();
	}

	if (!MainHUDWidget)
	{
		if (UWorld* RetryWorld = GetWorld())
		{
			RetryWorld->GetTimerManager().ClearTimer(DeferredMissionUIInitializationTimer);
			RetryWorld->GetTimerManager().SetTimer(
				DeferredMissionUIInitializationTimer,
				this,
				&ABondPlayerController::InitializeMissionObjectiveUI,
				0.1f,
				false
			);
		}

		UE_LOG(LogTemp, Warning, TEXT("[Mission UI] Main HUD not ready; retrying objective UI initialization."));
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission UI] Cannot initialize objective UI: missing world."));
		return;
	}

	UGameplayMissionSubsystem* MissionSubsystem = World->GetSubsystem<UGameplayMissionSubsystem>();

	if (!MissionSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission UI] Cannot initialize objective UI: missing mission subsystem."));
		return;
	}

	const TArray<UMissionObjective*>& ActiveObjectives = MissionSubsystem->GetActiveObjectives();

	MissionSubsystem->OnObjectiveProgressChanged.RemoveDynamic(
		this,
		&ABondPlayerController::HandleMissionObjectiveProgressChanged
	);

	MissionSubsystem->OnObjectiveProgressChanged.AddDynamic(
		this,
		&ABondPlayerController::HandleMissionObjectiveProgressChanged
	);

	MissionSubsystem->OnObjectiveCompleted.RemoveDynamic(
		this,
		&ABondPlayerController::HandleMissionObjectiveCompleted
	);

	MissionSubsystem->OnObjectiveCompleted.AddDynamic(
		this,
		&ABondPlayerController::HandleMissionObjectiveCompleted
	);

	UE_LOG(LogTemp, Log, TEXT("[Mission UI] Initializing %d objective rows."), ActiveObjectives.Num());

	for (const UMissionObjective* Objective : ActiveObjectives)
	{
		if (!Objective)
		{
			continue;
		}

		UE_LOG(
			LogTemp,
			Log,
			TEXT("[Mission UI] Objective=%s Status=%d Progress=%d/%d"),
			*Objective->GetObjectiveId().ToString(),
			static_cast<int32>(Objective->GetStatus()),
			Objective->GetCurrentProgress(),
			Objective->GetRequiredProgress()
		);
	}

	OnMissionObjectiveUIInitialized(ActiveObjectives);
}

void ABondPlayerController::ToggleMissionPanel()
{
	SetMissionPanelVisible(!bMissionPanelVisible);
}

void ABondPlayerController::SetMissionPanelVisible(bool bVisible)
{
	if (bMissionPanelVisible == bVisible)
	{
		return;
	}

	bMissionPanelVisible = bVisible;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[Mission UI] Mission panel visibility changed: %s"),
		bMissionPanelVisible ? TEXT("Visible") : TEXT("Hidden")
	);

	OnMissionPanelVisibilityChanged(bMissionPanelVisible);
}

bool ABondPlayerController::IsMissionPanelVisible() const
{
	return bMissionPanelVisible;
}

void ABondPlayerController::BindPossessedBondDelegates()
{
	if (!PossessedBond)
	{
		PossessedBond = Cast<AJamesBondCharacter>(GetPawn());
	}

	if (!PossessedBond)
	{
		return;
	}

	UBondHealthComponent* HealthComponent = PossessedBond->FindComponentByClass<UBondHealthComponent>();

	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.RemoveDynamic(
			this,
			&ABondPlayerController::HandleHealthChanged
		);

		HealthComponent->OnHealthChanged.AddDynamic(
			this,
			&ABondPlayerController::HandleHealthChanged
		);

		HealthComponent->OnDamageTaken.RemoveDynamic(
			this,
			&ABondPlayerController::HandleDamageTaken
		);

		HealthComponent->OnDamageTaken.AddDynamic(
			this,
			&ABondPlayerController::HandleDamageTaken
		);

		HealthComponent->OnRegenerationStateChanged.RemoveDynamic(
			this,
			&ABondPlayerController::HandleRegenerationStateChanged
		);

		HealthComponent->OnRegenerationStateChanged.AddDynamic(
			this,
			&ABondPlayerController::HandleRegenerationStateChanged
		);

		HealthComponent->OnDeath.RemoveDynamic(
			this,
			&ABondPlayerController::HandleDeath
		);

		HealthComponent->OnDeath.AddDynamic(
			this,
			&ABondPlayerController::HandleDeath
		);
	}

	UBondTimeSlowComponent* TimeSlowComponent = PossessedBond->FindComponentByClass<UBondTimeSlowComponent>();

	if (TimeSlowComponent)
	{
		TimeSlowComponent->OnMeterChanged.RemoveDynamic(
			this,
			&ABondPlayerController::HandleTimeSlowMeterChanged
		);

		TimeSlowComponent->OnMeterChanged.AddDynamic(
			this,
			&ABondPlayerController::HandleTimeSlowMeterChanged
		);

		TimeSlowComponent->OnTimeSlowStateChanged.RemoveDynamic(
			this,
			&ABondPlayerController::HandleTimeSlowStateChanged
		);

		TimeSlowComponent->OnTimeSlowStateChanged.AddDynamic(
			this,
			&ABondPlayerController::HandleTimeSlowStateChanged
		);
	}

	BindWeaponDelegates();
}

void ABondPlayerController::CreatePlayerWidgets()
{
	if (!IsLocalController())
	{
		return;
	}

	if (!MainHUDWidget && MainHUDWidgetClass)
	{
		MainHUDWidget = CreateWidget<UUserWidget>(this,MainHUDWidgetClass);

		if (MainHUDWidget)
		{
			MainHUDWidget->AddToViewport();
		}
	}

	if (!TimeSlowHUDWidget && TimeSlowHUDWidgetClass)
	{
		TimeSlowHUDWidget = CreateWidget<UUserWidget>(this,TimeSlowHUDWidgetClass);

		if (TimeSlowHUDWidget)
		{
			TimeSlowHUDWidget->AddToViewport();
		}
	}
}

void ABondPlayerController::ShowDeathWidget()
{
	if (!IsLocalController())
	{
		return;
	}

	if (!DeathWidget && DeathWidgetClass)
	{
		DeathWidget = CreateWidget<UUserWidget>(this, DeathWidgetClass);
	}

	if (DeathWidget && !DeathWidget->IsInViewport())
	{
		DeathWidget->AddToViewport(100);
	}

	SetPause(true);
	bShowMouseCursor = true;

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(
		DeathWidget ? DeathWidget->TakeWidget() : TSharedPtr<SWidget>()
	);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void ABondPlayerController::RestoreGameplayInput()
{
	SetPause(false);
	bShowMouseCursor = false;
	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
}

void ABondPlayerController::BindWeaponDelegates()
{
	if (!PossessedBond)
	{
		return;
	}

	UBondWeaponComponent* WeaponComponent = PossessedBond->FindComponentByClass<UBondWeaponComponent>();

	if (!WeaponComponent)
	{
		return;
	}

	ABondWeaponBase* EquippedWeapon = WeaponComponent->GetEquippedWeapon();

	if (!EquippedWeapon)
	{
		UWorld* World = GetWorld();

		if (World)
		{
			World->GetTimerManager().SetTimer(
				DeferredWeaponBindingTimer,
				this,
				&ABondPlayerController::BindWeaponDelegates,
				0.1f,
				false
			);
		}

		return;
	}

	EquippedWeapon->OnAmmoChanged.RemoveDynamic(
		this,
		&ABondPlayerController::HandleAmmoChanged
	);

	EquippedWeapon->OnAmmoChanged.AddDynamic(
		this,
		&ABondPlayerController::HandleAmmoChanged
	);

	HandleAmmoChanged(
		EquippedWeapon->GetMagazineAmmo(),
		EquippedWeapon->GetReserveAmmo()
	);
}

void ABondPlayerController::HandleHealthChanged(
	float CurrentHealth,
	float MaxHealth,
	float HealthPercent
)
{
	UpdateDamageState(
		CurrentHealth,
		MaxHealth,
		HealthPercent
	);
}

void ABondPlayerController::HandleDamageTaken(float DamageAmount)
{
	PlayDamageFlash(DamageAmount);
}

void ABondPlayerController::HandleRegenerationStateChanged(
	bool bIsRegenerating
)
{
	SetRegenerating(bIsRegenerating);
}

void ABondPlayerController::HandleDeath()
{
	if (UWorld* World = GetWorld(); World && DeathWidgetDelay > 0.0f)
	{
		World->GetTimerManager().ClearTimer(DeathWidgetTimer);
		World->GetTimerManager().SetTimer(
			DeathWidgetTimer,
			this,
			&ABondPlayerController::ShowDeathWidget,
			DeathWidgetDelay,
			false
		);
	}
	else
	{
		ShowDeathWidget();
	}

	HandleBondDeath();
}

void ABondPlayerController::HandleAmmoChanged(int32 MagazineAmmo,int32 ReserveAmmo)
{
	UpdateAmmo(MagazineAmmo, ReserveAmmo);
}

void ABondPlayerController::HandleTimeSlowMeterChanged(float CurrentMeter,float MaxMeter)
{
	UpdateTimeSlowMeter(CurrentMeter, MaxMeter);
}

void ABondPlayerController::HandleTimeSlowStateChanged(bool bIsActive)
{
	SetTimeSlowActive(bIsActive);
}

void ABondPlayerController::HandleMissionObjectiveProgressChanged(
	FName ObjectiveId,
	int32 CurrentProgress,
	int32 RequiredProgress
)
{
	UE_LOG(
		LogTemp,
		Log,
		TEXT("[Mission UI] Refreshing after progress Objective=%s Progress=%d/%d"),
		*ObjectiveId.ToString(),
		CurrentProgress,
		RequiredProgress
	);

	InitializeMissionObjectiveUI();
}

void ABondPlayerController::HandleMissionObjectiveCompleted(FName ObjectiveId)
{
	UE_LOG(
		LogTemp,
		Log,
		TEXT("[Mission UI] Refreshing after completion Objective=%s"),
		*ObjectiveId.ToString()
	);

	InitializeMissionObjectiveUI();
}
