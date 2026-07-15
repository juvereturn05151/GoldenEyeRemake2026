#include "BondPlayerController.h"

#include "../Characters/JamesBondCharacter.h"
#include "../Components/BondHealthComponent.h"
#include "../Components/BondTimeSlowComponent.h"
#include "../Components/BondWeaponComponent.h"
#include "../Weapons/BondWeaponBase.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"

namespace
{
UUserWidget* FindExistingPlayerWidget(
	UObject* WorldContextObject,
	TSubclassOf<UUserWidget> WidgetClass
)
{
	if (!WorldContextObject || !WidgetClass)
	{
		return nullptr;
	}

	TArray<UUserWidget*> ExistingWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
		WorldContextObject,
		ExistingWidgets,
		WidgetClass,
		true
	);

	return ExistingWidgets.IsEmpty() ? nullptr : ExistingWidgets[0];
}
}

ABondPlayerController::ABondPlayerController()
{
}

void ABondPlayerController::BeginPlay()
{
	Super::BeginPlay();

	CreatePlayerWidgets();
	BindPossessedBondDelegates();
}

void ABondPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

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

	UBondHealthComponent* HealthComponent =
		PossessedBond->FindComponentByClass<UBondHealthComponent>();

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

	UBondTimeSlowComponent* TimeSlowComponent =
		PossessedBond->FindComponentByClass<UBondTimeSlowComponent>();

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
		MainHUDWidget = FindExistingPlayerWidget(this, MainHUDWidgetClass);

		if (!MainHUDWidget)
		{
			MainHUDWidget = CreateWidget<UUserWidget>(
				this,
				MainHUDWidgetClass
			);
		}

		if (MainHUDWidget)
		{
			if (!MainHUDWidget->IsInViewport())
			{
				MainHUDWidget->AddToViewport();
			}
		}
	}

	if (!TimeSlowHUDWidget && TimeSlowHUDWidgetClass)
	{
		TimeSlowHUDWidget = FindExistingPlayerWidget(this, TimeSlowHUDWidgetClass);

		if (!TimeSlowHUDWidget)
		{
			TimeSlowHUDWidget = CreateWidget<UUserWidget>(
				this,
				TimeSlowHUDWidgetClass
			);
		}

		if (TimeSlowHUDWidget)
		{
			if (!TimeSlowHUDWidget->IsInViewport())
			{
				TimeSlowHUDWidget->AddToViewport();
			}
		}
	}
}

void ABondPlayerController::BindWeaponDelegates()
{
	if (!PossessedBond)
	{
		return;
	}

	UBondWeaponComponent* WeaponComponent =
		PossessedBond->FindComponentByClass<UBondWeaponComponent>();

	if (!WeaponComponent)
	{
		return;
	}

	ABondWeaponBase* EquippedWeapon =
		WeaponComponent->GetEquippedWeapon();

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
	HandleBondDeath();
}

void ABondPlayerController::HandleAmmoChanged(
	int32 MagazineAmmo,
	int32 ReserveAmmo
)
{
	UpdateAmmo(MagazineAmmo, ReserveAmmo);
}

void ABondPlayerController::HandleTimeSlowMeterChanged(
	float CurrentMeter,
	float MaxMeter
)
{
	UpdateTimeSlowMeter(CurrentMeter, MaxMeter);
}

void ABondPlayerController::HandleTimeSlowStateChanged(bool bIsActive)
{
	SetTimeSlowActive(bIsActive);
}
