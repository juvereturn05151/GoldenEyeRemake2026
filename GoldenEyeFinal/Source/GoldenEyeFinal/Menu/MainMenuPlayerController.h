#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainMenuPlayerController.generated.h"

class UMainMenuWidget;

UCLASS()
class GOLDENEYEFINAL_API AMainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMainMenuPlayerController();

	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void StartGame();

	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void QuitGame();

	UFUNCTION(BlueprintPure, Category = "Main Menu")
	FText GetMissionText() const;

	UFUNCTION(BlueprintPure, Category = "Main Menu")
	TArray<FText> GetControlLines() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|UI")
	TSubclassOf<UMainMenuWidget> MainMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Level")
	FName GameplayLevelName = TEXT("BunkerILevel");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Text", meta = (MultiLine = "true"))
	FText MissionText = FText::FromString(
		TEXT("Complete all mission objectives inside the bunker and escape the facility.\n\nObjectives may be completed in any order.")
	);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Text")
	TArray<FText> ControlLines;

private:
	UPROPERTY()
	TObjectPtr<UMainMenuWidget> MainMenuWidget;

	void CreateMainMenuWidget();
	void ConfigureMenuInput();
	void ClearPawnForMenu();
};
