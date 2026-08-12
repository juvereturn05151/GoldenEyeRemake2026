#include "WinningArea.h"

#include "../Characters/JamesBondCharacter.h"
#include "../Player/BondPlayerController.h"
#include "Components/BoxComponent.h"

AWinningArea::AWinningArea()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	SetRootComponent(TriggerBox);

	TriggerBox->SetBoxExtent(TriggerBoxExtent);
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
}

void AWinningArea::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerBox)
	{
		TriggerBox->SetBoxExtent(TriggerBoxExtent);
		TriggerBox->OnComponentBeginOverlap.AddDynamic(
			this,
			&AWinningArea::HandleTriggerBeginOverlap
		);
	}
}

void AWinningArea::ResetWinningArea()
{
	bHasTriggered = false;
}

void AWinningArea::HandleTriggerBeginOverlap(
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

	AJamesBondCharacter* Bond = Cast<AJamesBondCharacter>(OtherActor);

	if (!Bond)
	{
		return;
	}

	bHasTriggered = true;

	UE_LOG(LogTemp, Log, TEXT("[Winning Area] Bond entered winning area=%s"), *GetName());
	OnWinningAreaTriggered.Broadcast(Bond);

	if (ABondPlayerController* BondController = Cast<ABondPlayerController>(Bond->GetController()))
	{
		BondController->ShowWinWidget(bPauseGameOnWin);
	}
}
