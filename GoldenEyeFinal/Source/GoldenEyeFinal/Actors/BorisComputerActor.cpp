#include "BorisComputerActor.h"

#include "Components/SceneComponent.h"

ABorisComputerActor::ABorisComputerActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

bool ABorisComputerActor::ActivateComputer()
{
	if (bActivated)
	{
		UE_LOG(LogTemp, Log, TEXT("BORIS: Computer activation ignored because %s is already activated"), *GetNameSafe(this));
		return false;
	}

	bActivated = true;
	UE_LOG(LogTemp, Log, TEXT("BORIS: Computer activated"));
	OnComputerActivated.Broadcast(this);
	return true;
}

bool ABorisComputerActor::IsActivated() const
{
	return bActivated;
}
