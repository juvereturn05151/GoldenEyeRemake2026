#include "SurveillanceAlertComponent.h"

#include "Components/LightComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

USurveillanceAlertComponent::USurveillanceAlertComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USurveillanceAlertComponent::BeginPlay()
{
	Super::BeginPlay();

	AlertLight = ResolveAlertLight();
	CacheOriginalLightState();

	if (AlertLight && bHideAlertLightOnBeginPlay)
	{
		AlertLight->SetVisibility(false);
	}
}

void USurveillanceAlertComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopAlertFlash();

	Super::EndPlay(EndPlayReason);
}

void USurveillanceAlertComponent::SetAlertLight(ULightComponent* InAlertLight)
{
	if (bIsFlashing)
	{
		StopAlertFlash();
	}

	AlertLight = InAlertLight;
	CacheOriginalLightState();

	if (AlertLight && bHideAlertLightOnBeginPlay)
	{
		AlertLight->SetVisibility(false);
	}
}

ULightComponent* USurveillanceAlertComponent::GetAlertLight() const
{
	return AlertLight;
}

void USurveillanceAlertComponent::StartAlertFlash(float OverrideDuration)
{
	UWorld* World = GetWorld();

	AlertLight = ResolveAlertLight();

	if (!World || !AlertLight)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[Surveillance Alert] Missing alert light Owner=%s"),
			*GetNameSafe(GetOwner())
		);
		return;
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[Surveillance Alert] Starting Owner=%s Light=%s Duration=%.2f Interval=%.2f"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(AlertLight),
		OverrideDuration > 0.0f ? OverrideDuration : FlashDuration,
		FlashInterval
	);

	World->GetTimerManager().ClearTimer(FlashToggleTimer);
	World->GetTimerManager().ClearTimer(FlashStopTimer);

	bIsFlashing = true;
	ApplyAlertLightState(true);

	World->GetTimerManager().SetTimer(
		FlashToggleTimer,
		this,
		&USurveillanceAlertComponent::ToggleAlertLight,
		FlashInterval,
		true
	);

	const float Duration = OverrideDuration > 0.0f ? OverrideDuration : FlashDuration;

	if (Duration > 0.0f)
	{
		World->GetTimerManager().SetTimer(
			FlashStopTimer,
			this,
			&USurveillanceAlertComponent::StopAlertFlash,
			Duration,
			false
		);
	}
}

void USurveillanceAlertComponent::StopAlertFlash()
{
	UWorld* World = GetWorld();

	if (World)
	{
		World->GetTimerManager().ClearTimer(FlashToggleTimer);
		World->GetTimerManager().ClearTimer(FlashStopTimer);
	}

	if (!bIsFlashing)
	{
		return;
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[Surveillance Alert] Stopping Owner=%s Light=%s"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(AlertLight)
	);

	bIsFlashing = false;
	RestoreOriginalLightState();
}

bool USurveillanceAlertComponent::IsAlertFlashing() const
{
	return bIsFlashing;
}

ULightComponent* USurveillanceAlertComponent::ResolveAlertLight() const
{
	return AlertLight;
}

void USurveillanceAlertComponent::ToggleAlertLight()
{
	if (!AlertLight)
	{
		return;
	}

	ApplyAlertLightState(!AlertLight->IsVisible());
}

void USurveillanceAlertComponent::ApplyAlertLightState(bool bLightOn)
{
	if (!AlertLight)
	{
		return;
	}

	AlertLight->SetLightColor(AlertColor);
	AlertLight->SetIntensity(AlertIntensity);
	AlertLight->SetVisibility(bLightOn);
}

void USurveillanceAlertComponent::CacheOriginalLightState()
{
	if (!AlertLight)
	{
		return;
	}

	bOriginalVisibility = AlertLight->IsVisible();
	OriginalLightColor = AlertLight->GetLightColor();
	OriginalIntensity = AlertLight->Intensity;
}

void USurveillanceAlertComponent::RestoreOriginalLightState()
{
	if (!AlertLight)
	{
		return;
	}

	AlertLight->SetLightColor(OriginalLightColor);
	AlertLight->SetIntensity(OriginalIntensity);
	AlertLight->SetVisibility(bOriginalVisibility && !bHideAlertLightOnBeginPlay);
}
