#include "SurveillanceMissionComponent.h"

#include "../Mission/GameplayMissionSubsystem.h"

USurveillanceMissionComponent::USurveillanceMissionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USurveillanceMissionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bRegisterOnBeginPlay)
	{
		RegisterWithMissionSystem();
	}
}

void USurveillanceMissionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterFromMissionSystem();

	Super::EndPlay(EndPlayReason);
}

bool USurveillanceMissionComponent::RegisterWithMissionSystem()
{
	AActor* OwnerActor = GetOwner();

	if (!OwnerActor)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission] Surveillance register failed: missing owner"));
		return false;
	}

	if (bIsRegistered)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Mission] Surveillance register ignored: already registered actor=%s"), *GetNameSafe(OwnerActor));
		return true;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission] Missing world while registering actor=%s"), *GetNameSafe(OwnerActor));
		return false;
	}

	UGameplayMissionSubsystem* MissionSubsystem = World->GetSubsystem<UGameplayMissionSubsystem>();

	if (!MissionSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission] Missing subsystem while registering actor=%s"), *GetNameSafe(OwnerActor));
		return false;
	}

	bIsRegistered = MissionSubsystem->RegisterMissionActorWithIds(
		OwnerActor,
		MissionGroupId,
		MissionActorId,
		bAlreadyDestroyed
	);

	if (bIsRegistered)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[Mission] Surveillance camera ready actor=%s Group=%s"),
			*GetNameSafe(OwnerActor),
			*MissionGroupId.ToString()
		);

		FMissionEventData EventData;
		EventData.EventTag = TEXT("Camera.Registered");
		EventData.Target = OwnerActor;
		EventData.ContextId = MissionGroupId;
		EventData.Amount = 1;

		MissionSubsystem->BroadcastMissionEvent(EventData);
	}

	return bIsRegistered;
}

void USurveillanceMissionComponent::UnregisterFromMissionSystem()
{
	AActor* OwnerActor = GetOwner();

	if (!OwnerActor || !bIsRegistered)
	{
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission] Missing world while unregistering actor=%s"), *GetNameSafe(OwnerActor));
		return;
	}

	UGameplayMissionSubsystem* MissionSubsystem = World->GetSubsystem<UGameplayMissionSubsystem>();

	if (!MissionSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission] Missing subsystem while unregistering actor=%s"), *GetNameSafe(OwnerActor));
		return;
	}

	MissionSubsystem->UnregisterMissionActor(OwnerActor);
	bIsRegistered = false;
}

void USurveillanceMissionComponent::NotifyCameraDestroyed(AActor* EventInstigator)
{
	AActor* OwnerActor = GetOwner();

	if (!OwnerActor)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission] Camera destroyed notification failed: missing owner"));
		return;
	}

	if (bAlreadyDestroyed)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Mission] Duplicate destruction ignored=%s"), *GetNameSafe(OwnerActor));
		return;
	}

	bAlreadyDestroyed = true;

	if (!bIsRegistered)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Mission] Camera destroyed notification from unregistered actor=%s"), *GetNameSafe(OwnerActor));
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission] Missing world while notifying camera destroyed actor=%s"), *GetNameSafe(OwnerActor));
		return;
	}

	UGameplayMissionSubsystem* MissionSubsystem = World->GetSubsystem<UGameplayMissionSubsystem>();

	if (!MissionSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission] Missing subsystem while notifying camera destroyed actor=%s"), *GetNameSafe(OwnerActor));
		return;
	}

	FMissionEventData EventData;
	EventData.EventTag = TEXT("Camera.Destroyed");
	EventData.Instigator = EventInstigator;
	EventData.Target = OwnerActor;
	EventData.ContextId = MissionGroupId;
	EventData.Amount = 1;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[Mission] Camera destroyed notified actor=%s Group=%s Instigator=%s"),
		*GetNameSafe(OwnerActor),
		*MissionGroupId.ToString(),
		*GetNameSafe(EventInstigator)
	);

	MissionSubsystem->BroadcastMissionEvent(EventData);
}

bool USurveillanceMissionComponent::IsAlreadyDestroyed() const
{
	return bAlreadyDestroyed;
}

FName USurveillanceMissionComponent::GetMissionGroupId_Implementation() const
{
	return MissionGroupId;
}

FName USurveillanceMissionComponent::GetMissionActorId_Implementation() const
{
	return MissionActorId;
}

bool USurveillanceMissionComponent::IsMissionActorCompleted_Implementation() const
{
	return bAlreadyDestroyed;
}
