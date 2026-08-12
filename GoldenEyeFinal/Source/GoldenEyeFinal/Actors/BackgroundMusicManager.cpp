#include "BackgroundMusicManager.h"

#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

ABackgroundMusicManager::ABackgroundMusicManager()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	MusicComponentA = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicComponentA"));
	MusicComponentA->SetupAttachment(SceneRoot);
	MusicComponentA->bAutoActivate = false;

	MusicComponentB = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicComponentB"));
	MusicComponentB->SetupAttachment(SceneRoot);
	MusicComponentB->bAutoActivate = false;

	ActiveMusicComponent = MusicComponentA;
	PendingMusicComponent = MusicComponentB;
}

void ABackgroundMusicManager::BeginPlay()
{
	Super::BeginPlay();

	ActiveMusicComponent = MusicComponentA;
	PendingMusicComponent = MusicComponentB;

	if (bPlayStartingMusicOnBeginPlay && StartingMusic)
	{
		FadeToMusic(StartingMusic, StartingMusicState, 0.0f);
	}
}

void ABackgroundMusicManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MusicFadeTimer);
	}

	Super::EndPlay(EndPlayReason);
}

void ABackgroundMusicManager::PlayMusic(USoundBase* NewMusic, FName NewMusicState)
{
	FadeToMusic(NewMusic, NewMusicState, DefaultFadeTime);
}

void ABackgroundMusicManager::FadeToMusic(USoundBase* NewMusic, FName NewMusicState, float FadeTime)
{
	if (!NewMusic)
	{
		StopMusic(FadeTime);
		return;
	}

	if (NewMusicState != NAME_None && (CurrentMusicState == NewMusicState || PendingMusicState == NewMusicState))
	{
		return;
	}

	UWorld* World = GetWorld();

	if (World)
	{
		World->GetTimerManager().ClearTimer(MusicFadeTimer);
	}

	PendingMusic = NewMusic;
	PendingMusicState = NewMusicState;

	if (!PendingMusicComponent)
	{
		PendingMusicComponent = MusicComponentB;
	}

	if (!ActiveMusicComponent)
	{
		ActiveMusicComponent = MusicComponentA;
	}

	if (PendingMusicComponent)
	{
		PendingMusicComponent->Stop();
		PendingMusicComponent->SetSound(NewMusic);
		PendingMusicComponent->FadeIn(FadeTime, MusicVolume);
	}

	if (ActiveMusicComponent && ActiveMusicComponent->IsPlaying())
	{
		ActiveMusicComponent->FadeOut(FadeTime, 0.0f);
	}

	if (!World || FadeTime <= 0.0f)
	{
		CommitPendingMusic();
		return;
	}

	World->GetTimerManager().SetTimer(
		MusicFadeTimer,
		this,
		&ABackgroundMusicManager::CommitPendingMusic,
		FadeTime,
		false
	);
}

void ABackgroundMusicManager::StopMusic(float FadeTime)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MusicFadeTimer);
	}

	if (ActiveMusicComponent && ActiveMusicComponent->IsPlaying())
	{
		ActiveMusicComponent->FadeOut(FadeTime, 0.0f);
	}

	if (PendingMusicComponent && PendingMusicComponent->IsPlaying())
	{
		PendingMusicComponent->FadeOut(FadeTime, 0.0f);
	}

	CurrentMusicState = NAME_None;
	PendingMusicState = NAME_None;
	PendingMusic = nullptr;
}

FName ABackgroundMusicManager::GetCurrentMusicState() const
{
	return CurrentMusicState;
}

void ABackgroundMusicManager::CommitPendingMusic()
{
	if (!PendingMusicComponent)
	{
		return;
	}

	UAudioComponent* PreviousActiveComponent = ActiveMusicComponent;
	ActiveMusicComponent = PendingMusicComponent;
	PendingMusicComponent = PreviousActiveComponent;

	CurrentMusicState = PendingMusicState;
	PendingMusicState = NAME_None;

	if (PendingMusicComponent)
	{
		PendingMusicComponent->Stop();
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[Music] Changed music state=%s Music=%s"),
		*CurrentMusicState.ToString(),
		*GetNameSafe(PendingMusic)
	);

	OnMusicChanged.Broadcast(CurrentMusicState, PendingMusic);
	PendingMusic = nullptr;
}
