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

	FGoldenEyeEventObjectiveDefinition CopyObjective;
	CopyObjective.ObjectiveId = TEXT("CopyGoldenEyeKey");
	CopyObjective.DisplayName = NSLOCTEXT("GoldenEyeMission", "CopyGoldenEyeKey", "Copy GoldenEye Key and Leave Original");
	CopyObjective.Description = NSLOCTEXT(
		"GoldenEyeMission",
		"CopyGoldenEyeKeyDescription",
		"Use the copy device long enough to duplicate the GoldenEye key data without taking the original."
	);
	CopyObjective.RequiredEventTag = TEXT("Copy.Completed");
	CopyObjective.RequiredContextId = TEXT("GoldenEyeKey");
	CopyObjective.RequiredProgress = 1;
	EventObjectives.Add(CopyObjective);

	FGoldenEyeEventObjectiveDefinition BorisComputerObjective;
	BorisComputerObjective.ObjectiveId = TEXT("BorisActivateMainComputer");
	BorisComputerObjective.DisplayName = NSLOCTEXT("GoldenEyeMission", "BorisActivateMainComputer", "Have Boris Activate Main Computer");
	BorisComputerObjective.Description = NSLOCTEXT(
		"GoldenEyeMission",
		"BorisActivateMainComputerDescription",
		"Escort Boris through the sequence so he activates the main computer terminal."
	);
	BorisComputerObjective.RequiredEventTag = TEXT("Computer.Activated");
	BorisComputerObjective.RequiredContextId = TEXT("MainComputer");
	BorisComputerObjective.RequiredProgress = 1;
	EventObjectives.Add(BorisComputerObjective);

	FGoldenEyeEventObjectiveDefinition DownloadDataObjective;
	DownloadDataObjective.ObjectiveId = TEXT("DownloadMainComputerData");
	DownloadDataObjective.DisplayName = NSLOCTEXT("GoldenEyeMission", "DownloadMainComputerData", "Download Data From Main Computer");
	DownloadDataObjective.Description = NSLOCTEXT(
		"GoldenEyeMission",
		"DownloadMainComputerDataDescription",
		"After Boris activates the main computer, interact with the terminal to download the data."
	);
	DownloadDataObjective.RequiredEventTag = TEXT("Computer.DataDownloaded");
	DownloadDataObjective.RequiredContextId = TEXT("MainComputer");
	DownloadDataObjective.RequiredProgress = 1;
	EventObjectives.Add(DownloadDataObjective);
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
