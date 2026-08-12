#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UPanelWidget;
class UTextBlock;
class UVerticalBox;
class UWidget;
class SButton;
class SWidgetSwitcher;

UCLASS()
class GOLDENEYEFINAL_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void ShowStartPage();

	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void ShowHowToPlayPage();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> StartPage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> HowToPlayPage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> StartButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> HowToPlayButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> ExitButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> PlayButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> BackButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MissionTextBlock;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> ControlsContainer;

private:
	UFUNCTION()
	void HandleStartClicked();

	UFUNCTION()
	void HandleHowToPlayClicked();

	UFUNCTION()
	void HandleExitClicked();

	UFUNCTION()
	void HandlePlayClicked();

	UFUNCTION()
	void HandleBackClicked();

	void BindButtonDelegates();
	void BuildNativeWidgetTree();
	TSharedRef<SWidget> BuildNativeSlateWidget();
	TSharedRef<SWidget> BuildNativeStartPage();
	TSharedRef<SWidget> BuildNativeHowToPlayPage();
	void PopulateMenuText();
	void PopulateControlLines(const TArray<FText>& ControlLines);
	UTextBlock* CreateTextBlock(const FText& Text, int32 FontSize, const FLinearColor& Color) const;
	UButton* CreateMenuButton(const FText& Text) const;
	void AddTextToVerticalBox(UVerticalBox* TargetBox, UTextBlock* TextBlock, const FMargin& InPadding) const;
	void AddWidgetToVerticalBox(UVerticalBox* TargetBox, UWidget* Widget, const FMargin& InPadding) const;

	FReply HandleStartSlateClicked();
	FReply HandleHowToPlaySlateClicked();
	FReply HandleExitSlateClicked();
	FReply HandlePlaySlateClicked();
	FReply HandleBackSlateClicked();

	TSharedPtr<SWidgetSwitcher> NativePageSwitcher;
	TSharedPtr<SButton> NativeStartButton;
	TSharedPtr<SButton> NativePlayButton;
};
