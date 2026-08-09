#include "SurveillanceAlertComponent.h"

#include "Components/AudioComponent.h"
#include "Components/LightComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "EnemySpawnerComponent.h"
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

	if (bBindLinkedSpawnersOnBeginPlay)
	{
		BindLinkedEnemySpawners();
	}
}

void USurveillanceAlertComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopAlertFlash();
	UnbindLinkedEnemySpawners();

	if (
		EndPlayReason == EEndPlayReason::Destroyed &&
		bDestroyLinkedSpawnersOnCameraDestroyed
		)
	{
		DestroyLinkedEnemySpawners();
	}

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
	PlayAlertSound();

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

	StopAlertSound();

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

void USurveillanceAlertComponent::BindLinkedEnemySpawners()
{
	for (UEnemySpawnerComponent* LinkedEnemySpawner : LinkedEnemySpawners)
	{
		if (!LinkedEnemySpawner)
		{
			continue;
		}

		LinkedEnemySpawner->OnBondEnteredSpawner.RemoveDynamic(
			this,
			&USurveillanceAlertComponent::HandleLinkedSpawnerTriggered
		);
		LinkedEnemySpawner->OnBondEnteredSpawner.AddDynamic(
			this,
			&USurveillanceAlertComponent::HandleLinkedSpawnerTriggered
		);

		LinkedEnemySpawner->OnSpawnerActivated.RemoveDynamic(
			this,
			&USurveillanceAlertComponent::HandleLinkedSpawnerTriggered
		);
		LinkedEnemySpawner->OnSpawnerActivated.AddDynamic(
			this,
			&USurveillanceAlertComponent::HandleLinkedSpawnerTriggered
		);
	}
}

ULightComponent* USurveillanceAlertComponent::ResolveAlertLight() const
{
	return AlertLight;
}

void USurveillanceAlertComponent::UnbindLinkedEnemySpawners()
{
	for (UEnemySpawnerComponent* LinkedEnemySpawner : LinkedEnemySpawners)
	{
		if (!LinkedEnemySpawner)
		{
			continue;
		}

		LinkedEnemySpawner->OnBondEnteredSpawner.RemoveDynamic(
			this,
			&USurveillanceAlertComponent::HandleLinkedSpawnerTriggered
		);

		LinkedEnemySpawner->OnSpawnerActivated.RemoveDynamic(
			this,
			&USurveillanceAlertComponent::HandleLinkedSpawnerTriggered
		);
	}
}

void USurveillanceAlertComponent::DestroyLinkedEnemySpawners()
{
	AActor* OwnerActor = GetOwner();

	for (UEnemySpawnerComponent* LinkedEnemySpawner : LinkedEnemySpawners)
	{
		if (!LinkedEnemySpawner)
		{
			continue;
		}

		AActor* SpawnerOwner = LinkedEnemySpawner->GetOwner();

		if (SpawnerOwner && SpawnerOwner != OwnerActor)
		{
			SpawnerOwner->Destroy();
			continue;
		}

		LinkedEnemySpawner->DestroyComponent();
	}
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

void USurveillanceAlertComponent::PlayAlertSound()
{
	UWorld* World = GetWorld();

	if (!World || !AlertSound)
	{
		return;
	}

	StopAlertSound();

	if (bPlayAlertSound2D)
	{
		ActiveAlertAudioComponent = UGameplayStatics::SpawnSound2D(
			World,
			AlertSound,
			AlertSoundVolume,
			AlertSoundPitch,
			0.0f,
			nullptr,
			false,
			false
		);
		return;
	}

	const AActor* OwnerActor = GetOwner();

	ActiveAlertAudioComponent = UGameplayStatics::SpawnSoundAtLocation(
		World,
		AlertSound,
		OwnerActor ? OwnerActor->GetActorLocation() : FVector::ZeroVector,
		FRotator::ZeroRotator,
		AlertSoundVolume,
		AlertSoundPitch,
		0.0f,
		nullptr,
		nullptr,
		false
	);
}

void USurveillanceAlertComponent::StopAlertSound()
{
	if (!ActiveAlertAudioComponent)
	{
		return;
	}

	ActiveAlertAudioComponent->Stop();
	ActiveAlertAudioComponent->DestroyComponent();
	ActiveAlertAudioComponent = nullptr;
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

void USurveillanceAlertComponent::HandleLinkedSpawnerTriggered(AActor* BondActor)
{
	StartAlertFlash();
}
