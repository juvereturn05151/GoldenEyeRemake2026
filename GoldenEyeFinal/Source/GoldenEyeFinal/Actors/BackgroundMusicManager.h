#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BackgroundMusicManager.generated.h"

class UAudioComponent;
class USceneComponent;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FBackgroundMusicChangedSignature,
	FName,
	MusicState,
	USoundBase*,
	Music
);

UCLASS()
class GOLDENEYEFINAL_API ABackgroundMusicManager : public AActor
{
	GENERATED_BODY()

public:
	ABackgroundMusicManager();

	UFUNCTION(BlueprintCallable, Category = "Music")
	void PlayMusic(USoundBase* NewMusic, FName NewMusicState);

	UFUNCTION(BlueprintCallable, Category = "Music")
	void FadeToMusic(USoundBase* NewMusic, FName NewMusicState, float FadeTime);

	UFUNCTION(BlueprintCallable, Category = "Music")
	void StopMusic(float FadeTime);

	UFUNCTION(BlueprintPure, Category = "Music")
	FName GetCurrentMusicState() const;

	UPROPERTY(BlueprintAssignable, Category = "Music")
	FBackgroundMusicChangedSignature OnMusicChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Music|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Music|Components")
	TObjectPtr<UAudioComponent> MusicComponentA;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Music|Components")
	TObjectPtr<UAudioComponent> MusicComponentB;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music")
	bool bPlayStartingMusicOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music")
	TObjectPtr<USoundBase> StartingMusic;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music")
	FName StartingMusicState = TEXT("Exploration");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music", meta = (ClampMin = "0.0"))
	float DefaultFadeTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music", meta = (ClampMin = "0.0"))
	float MusicVolume = 1.0f;

private:
	void CommitPendingMusic();

	UPROPERTY()
	TObjectPtr<UAudioComponent> ActiveMusicComponent;

	UPROPERTY()
	TObjectPtr<UAudioComponent> PendingMusicComponent;

	UPROPERTY()
	TObjectPtr<USoundBase> PendingMusic;

	FName CurrentMusicState = NAME_None;
	FName PendingMusicState = NAME_None;
	FTimerHandle MusicFadeTimer;
};
