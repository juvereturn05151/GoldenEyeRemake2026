#include "JojoBook.h"

#include "../Characters/JamesBondCharacter.h"
#include "BackgroundMusicManager.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "Sound/SoundBase.h"

AJojoBook::AJojoBook()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	BookMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BookMesh"));
	BookMesh->SetupAttachment(SceneRoot);
	BookMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BookMesh->SetCollisionObjectType(ECC_WorldDynamic);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(SceneRoot);
	TriggerBox->SetBoxExtent(FVector(80.0f, 80.0f, 80.0f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AJojoBook::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(
		this,
		&AJojoBook::HandleTriggerBeginOverlap
	);
}

void AJojoBook::HandleTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (bTriggerOnlyOnce && bHasTriggered)
	{
		return;
	}

	AJamesBondCharacter* BondCharacter = Cast<AJamesBondCharacter>(OtherActor);
	if (!BondCharacter)
	{
		return;
	}

	if (!JojoSong)
	{
		UE_LOG(LogTemp, Warning, TEXT("[JojoBook] %s cannot change music: JojoSong is not assigned"), *GetName());
		return;
	}

	ABackgroundMusicManager* ResolvedMusicManager = ResolveMusicManager();
	if (!ResolvedMusicManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("[JojoBook] %s cannot change music: MusicManager not found"), *GetName());
		return;
	}

	bHasTriggered = true;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[JojoBook] %s triggered by %s. Changing music to %s"),
		*GetName(),
		*BondCharacter->GetName(),
		*GetNameSafe(JojoSong)
	);

	ResolvedMusicManager->FadeToMusic(JojoSong, JojoMusicState, FadeTime);
}

ABackgroundMusicManager* AJojoBook::ResolveMusicManager() const
{
	if (MusicManager)
	{
		return MusicManager;
	}

	if (!bFindMusicManagerIfMissing)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<ABackgroundMusicManager> It(World); It; ++It)
	{
		return *It;
	}

	return nullptr;
}
