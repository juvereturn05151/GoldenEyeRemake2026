#include "GoldenEyeMissionInitializer.h"

#include "../Mission/GameplayMissionSubsystem.h"
#include "../Player/BondPlayerController.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AGoldenEyeMissionInitializer::AGoldenEyeMissionInitializer()
{
	PrimaryActorTick.bCanEverTick = false;

	FGoldenEyeSurveillanceObjectiveDefinition SurveillanceObjective;
	SurveillanceObjective.ObjectiveId = TEXT("DestroySurveillanceCameras");
	SurveillanceObjective.DisplayName = NSLOCTEXT("GoldenEyeMission", "DestroySurveillanceCameras", "Destroy Surveillance Cameras");
	SurveillanceObjective.Description = NSLOCTEXT(
		"GoldenEyeMission",
		"DestroySurveillanceCamerasDescription",
		"Find and destroy the active surveillance cameras before they can track Bond's movement."
	);
	SurveillanceObjective.TargetGroupId = TEXT("SurveillanceCameras");
	SurveillanceObjectives.Add(SurveillanceObjective);

	FGoldenEyeEventObjectiveDefinition PhotoObjective;
	PhotoObjective.ObjectiveId = TEXT("PhotographGoldenEye");
	PhotoObjective.DisplayName = NSLOCTEXT("GoldenEyeMission", "PhotographGoldenEye", "Photograph Main Video Screen");
	PhotoObjective.Description = NSLOCTEXT(
		"GoldenEyeMission",
		"PhotographGoldenEyeDescription",
		"Stand in the marked photo zone, face the main video screen, and take a clear photograph."
	);
	PhotoObjective.RequiredEventTag = TEXT("Photo.Taken");
	PhotoObjective.RequiredContextId = TEXT("GoldenEyePhoto");
	PhotoObjective.RequiredProgress = 1;
	EventObjectives.Add(PhotoObjective);
}

void AGoldenEyeMissionInitializer::BeginPlay()
{
	Super::BeginPlay();

	if (bInitializeOnBeginPlay)
	{
		InitializeMissionAndUI();
	}
}

void AGoldenEyeMissionInitializer::InitializeMissionAndUI()
{
	UWorld* World = GetWorld();

	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission] InitializeMissionAndUI failed: missing world."));
		return;
	}

	UGameplayMissionSubsystem* MissionSubsystem = World->GetSubsystem<UGameplayMissionSubsystem>();

	if (!MissionSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[Mission] InitializeMissionAndUI failed: missing mission subsystem."));
		return;
	}

	for (const FGoldenEyeSurveillanceObjectiveDefinition& ObjectiveDefinition : SurveillanceObjectives)
	{
		if (ObjectiveDefinition.ObjectiveId == NAME_None || ObjectiveDefinition.TargetGroupId == NAME_None)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[Mission] Skipped surveillance objective with missing ObjectiveId or TargetGroupId.")
			);
			continue;
		}

		MissionSubsystem->StartSurveillanceObjective(
			ObjectiveDefinition.ObjectiveId,
			ObjectiveDefinition.DisplayName,
			ObjectiveDefinition.Description,
			ObjectiveDefinition.TargetGroupId
		);
	}

	for (const FGoldenEyeEventObjectiveDefinition& ObjectiveDefinition : EventObjectives)
	{
		if (ObjectiveDefinition.ObjectiveId == NAME_None || ObjectiveDefinition.RequiredEventTag == NAME_None)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[Mission] Skipped objective with missing ObjectiveId or RequiredEventTag.")
			);
			continue;
		}

		MissionSubsystem->StartEventObjective(
			ObjectiveDefinition.ObjectiveId,
			ObjectiveDefinition.DisplayName,
			ObjectiveDefinition.Description,
			ObjectiveDefinition.RequiredEventTag,
			ObjectiveDefinition.RequiredContextId,
			ObjectiveDefinition.RequiredProgress
		);
	}

	MissionSubsystem->StartMission();

	ABondPlayerController* BondController = Cast<ABondPlayerController>(
		UGameplayStatics::GetPlayerController(World, 0)
	);

	if (BondController)
	{
		BondController->InitializeMissionObjectiveUI();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Mission UI] InitializeMissionAndUI could not find BP_BondPlayerController."));
	}
}
