#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JojoBook.generated.h"

class ABackgroundMusicManager;
class AJamesBondCharacter;
class UBoxComponent;
class USceneComponent;
class USoundBase;
class UStaticMeshComponent;

UCLASS()
class GOLDENEYEFINAL_API AJojoBook : public AActor
{
	GENERATED_BODY()

public:
	AJojoBook();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Jojo Book|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Jojo Book|Components")
	TObjectPtr<UStaticMeshComponent> BookMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Jojo Book|Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jojo Book|Music")
	TObjectPtr<ABackgroundMusicManager> MusicManager;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jojo Book|Music")
	TObjectPtr<USoundBase> JojoSong;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jojo Book|Music")
	FName JojoMusicState = TEXT("Jojo");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jojo Book|Music", meta = (ClampMin = "0.0"))
	float FadeTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jojo Book|Music")
	bool bFindMusicManagerIfMissing = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jojo Book|Music")
	bool bTriggerOnlyOnce = true;

private:
	UFUNCTION()
	void HandleTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	ABackgroundMusicManager* ResolveMusicManager() const;

	bool bHasTriggered = false;
};
