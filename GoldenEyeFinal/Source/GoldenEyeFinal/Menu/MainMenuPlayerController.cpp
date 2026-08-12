#include "MainMenuPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MainMenuWidget.h"

AMainMenuPlayerController::AMainMenuPlayerController()
{
	bShowMouseCursor = true;
	MainMenuWidgetClass = UMainMenuWidget::StaticClass();

	ControlLines = {
		FText::FromString(TEXT("WASD                 Move")),
		FText::FromString(TEXT("Mouse                Look")),
		FText::FromString(TEXT("Left Mouse           Fire")),
		FText::FromString(TEXT("Right Mouse          Slow Motion")),
		FText::FromString(TEXT("R                    Reload")),
		FText::FromString(TEXT("Space                Jump")),
		FText::FromString(TEXT("P                    Interact / Download Data"))
	};
}

void AMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ClearPawnForMenu();
	CreateMainMenuWidget();
	ConfigureMenuInput();
}

void AMainMenuPlayerController::StartGame()
{
	if (GameplayLevelName.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("[Main Menu] Cannot start game: GameplayLevelName is not assigned."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[Main Menu] Loading gameplay level: %s"), *GameplayLevelName.ToString());
	UGameplayStatics::OpenLevel(this, GameplayLevelName);
}

void AMainMenuPlayerController::QuitGame()
{
	UKismetSystemLibrary::QuitGame(
		this,
		this,
		EQuitPreference::Quit,
		false
	);
}

FText AMainMenuPlayerController::GetMissionText() const
{
	return MissionText;
}

TArray<FText> AMainMenuPlayerController::GetControlLines() const
{
	return ControlLines;
}

void AMainMenuPlayerController::CreateMainMenuWidget()
{
	if (!IsLocalController())
	{
		return;
	}

	if (!MainMenuWidgetClass)
	{
		MainMenuWidgetClass = UMainMenuWidget::StaticClass();
	}

	MainMenuWidget = CreateWidget<UMainMenuWidget>(this, MainMenuWidgetClass);

	if (!MainMenuWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("[Main Menu] Failed to create main menu widget."));
		return;
	}

	MainMenuWidget->AddToViewport(0);
	MainMenuWidget->SetKeyboardFocus();
}

void AMainMenuPlayerController::ConfigureMenuInput()
{
	bShowMouseCursor = true;
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);

	FInputModeUIOnly InputMode;

	if (MainMenuWidget)
	{
		InputMode.SetWidgetToFocus(MainMenuWidget->TakeWidget());
	}

	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void AMainMenuPlayerController::ClearPawnForMenu()
{
	if (APawn* ControlledPawn = GetPawn())
	{
		UnPossess();
		ControlledPawn->Destroy();
	}
}
