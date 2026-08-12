#include "MainMenuWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"
#include "MainMenuPlayerController.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<SWidget> UMainMenuWidget::RebuildWidget()
{
	if (WidgetTree && WidgetTree->RootWidget)
	{
		return Super::RebuildWidget();
	}

	return BuildNativeSlateWidget();
}

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);

	if (WidgetTree && (!StartPage || !HowToPlayPage || !StartButton || !HowToPlayButton || !ExitButton || !PlayButton || !BackButton))
	{
		BuildNativeWidgetTree();
	}

	BindButtonDelegates();
	PopulateMenuText();
	ShowStartPage();
}

FReply UMainMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape && HowToPlayPage && HowToPlayPage->IsVisible())
	{
		ShowStartPage();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UMainMenuWidget::ShowStartPage()
{
	if (NativePageSwitcher)
	{
		NativePageSwitcher->SetActiveWidgetIndex(0);
	}

	if (StartPage)
	{
		StartPage->SetVisibility(ESlateVisibility::Visible);
	}

	if (HowToPlayPage)
	{
		HowToPlayPage->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (StartButton)
	{
		StartButton->SetKeyboardFocus();
	}

	if (NativeStartButton && FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().SetKeyboardFocus(NativeStartButton, EFocusCause::SetDirectly);
	}
}

void UMainMenuWidget::ShowHowToPlayPage()
{
	if (NativePageSwitcher)
	{
		NativePageSwitcher->SetActiveWidgetIndex(1);
	}

	if (StartPage)
	{
		StartPage->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (HowToPlayPage)
	{
		HowToPlayPage->SetVisibility(ESlateVisibility::Visible);
	}

	if (PlayButton)
	{
		PlayButton->SetKeyboardFocus();
	}

	if (NativePlayButton && FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().SetKeyboardFocus(NativePlayButton, EFocusCause::SetDirectly);
	}
}

void UMainMenuWidget::HandleStartClicked()
{
	if (AMainMenuPlayerController* MenuController = GetOwningPlayer<AMainMenuPlayerController>())
	{
		MenuController->StartGame();
	}
}

void UMainMenuWidget::HandleHowToPlayClicked()
{
	ShowHowToPlayPage();
}

void UMainMenuWidget::HandleExitClicked()
{
	if (AMainMenuPlayerController* MenuController = GetOwningPlayer<AMainMenuPlayerController>())
	{
		MenuController->QuitGame();
	}
}

void UMainMenuWidget::HandlePlayClicked()
{
	HandleStartClicked();
}

void UMainMenuWidget::HandleBackClicked()
{
	ShowStartPage();
}

void UMainMenuWidget::BindButtonDelegates()
{
	if (StartButton)
	{
		StartButton->OnClicked.RemoveDynamic(this, &UMainMenuWidget::HandleStartClicked);
		StartButton->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleStartClicked);
	}

	if (HowToPlayButton)
	{
		HowToPlayButton->OnClicked.RemoveDynamic(this, &UMainMenuWidget::HandleHowToPlayClicked);
		HowToPlayButton->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleHowToPlayClicked);
	}

	if (ExitButton)
	{
		ExitButton->OnClicked.RemoveDynamic(this, &UMainMenuWidget::HandleExitClicked);
		ExitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleExitClicked);
	}

	if (PlayButton)
	{
		PlayButton->OnClicked.RemoveDynamic(this, &UMainMenuWidget::HandlePlayClicked);
		PlayButton->OnClicked.AddDynamic(this, &UMainMenuWidget::HandlePlayClicked);
	}

	if (BackButton)
	{
		BackButton->OnClicked.RemoveDynamic(this, &UMainMenuWidget::HandleBackClicked);
		BackButton->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleBackClicked);
	}
}

void UMainMenuWidget::BuildNativeWidgetTree()
{
	if (!WidgetTree)
	{
		return;
	}

	UBorder* RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("NativeRootBorder"));
	RootBorder->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.96f));
	WidgetTree->RootWidget = RootBorder;

	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("NativeRootBox"));
	RootBorder->SetContent(RootBox);

	UVerticalBox* NativeStartPage = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StartPage"));
	StartPage = NativeStartPage;
	AddWidgetToVerticalBox(RootBox, NativeStartPage, FMargin(0.0f, 120.0f, 0.0f, 0.0f));

	AddTextToVerticalBox(
		NativeStartPage,
		CreateTextBlock(FText::FromString(TEXT("GOLDENEYE")), 72, FLinearColor(1.0f, 0.84f, 0.18f, 1.0f)),
		FMargin(0.0f, 0.0f, 0.0f, 48.0f)
	);

	StartButton = CreateMenuButton(FText::FromString(TEXT("START")));
	HowToPlayButton = CreateMenuButton(FText::FromString(TEXT("HOW TO PLAY")));
	ExitButton = CreateMenuButton(FText::FromString(TEXT("EXIT")));

	AddWidgetToVerticalBox(NativeStartPage, StartButton, FMargin(0.0f, 8.0f));
	AddWidgetToVerticalBox(NativeStartPage, HowToPlayButton, FMargin(0.0f, 8.0f));
	AddWidgetToVerticalBox(NativeStartPage, ExitButton, FMargin(0.0f, 8.0f));

	UVerticalBox* NativeHowToPlayPage = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("HowToPlayPage"));
	HowToPlayPage = NativeHowToPlayPage;
	AddWidgetToVerticalBox(RootBox, NativeHowToPlayPage, FMargin(0.0f, 80.0f, 0.0f, 0.0f));

	AddTextToVerticalBox(
		NativeHowToPlayPage,
		CreateTextBlock(FText::FromString(TEXT("HOW TO PLAY")), 48, FLinearColor(1.0f, 0.84f, 0.18f, 1.0f)),
		FMargin(0.0f, 0.0f, 0.0f, 28.0f)
	);

	AddTextToVerticalBox(
		NativeHowToPlayPage,
		CreateTextBlock(FText::FromString(TEXT("MISSION")), 28, FLinearColor::White),
		FMargin(0.0f, 0.0f, 0.0f, 8.0f)
	);

	MissionTextBlock = CreateTextBlock(FText::GetEmpty(), 22, FLinearColor(0.86f, 0.86f, 0.86f, 1.0f));
	MissionTextBlock->SetAutoWrapText(true);
	AddTextToVerticalBox(NativeHowToPlayPage, MissionTextBlock, FMargin(260.0f, 0.0f, 260.0f, 26.0f));

	AddTextToVerticalBox(
		NativeHowToPlayPage,
		CreateTextBlock(FText::FromString(TEXT("CONTROLS")), 28, FLinearColor::White),
		FMargin(0.0f, 0.0f, 0.0f, 8.0f)
	);

	ControlsContainer = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ControlsContainer"));
	AddWidgetToVerticalBox(NativeHowToPlayPage, ControlsContainer, FMargin(0.0f, 0.0f, 0.0f, 28.0f));

	PlayButton = CreateMenuButton(FText::FromString(TEXT("PLAY")));
	BackButton = CreateMenuButton(FText::FromString(TEXT("BACK")));

	AddWidgetToVerticalBox(NativeHowToPlayPage, PlayButton, FMargin(0.0f, 8.0f));
	AddWidgetToVerticalBox(NativeHowToPlayPage, BackButton, FMargin(0.0f, 8.0f));
}

TSharedRef<SWidget> UMainMenuWidget::BuildNativeSlateWidget()
{
	NativeStartButton.Reset();
	NativePlayButton.Reset();

	return SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.96f))
		[
			SAssignNew(NativePageSwitcher, SWidgetSwitcher)
			+ SWidgetSwitcher::Slot()
			[
				BuildNativeStartPage()
			]
			+ SWidgetSwitcher::Slot()
			[
				BuildNativeHowToPlayPage()
			]
		];
}

TSharedRef<SWidget> UMainMenuWidget::BuildNativeStartPage()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 120.0f, 0.0f, 48.0f)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("GOLDENEYE")))
			.Justification(ETextJustify::Center)
			.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.84f, 0.18f, 1.0f)))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 72))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 8.0f)
		.HAlign(HAlign_Center)
		[
			SAssignNew(NativeStartButton, SButton)
			.OnClicked(BIND_UOBJECT_DELEGATE(FOnClicked, HandleStartSlateClicked))
			.ContentPadding(FMargin(22.0f, 10.0f))
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("START")))
				.MinDesiredWidth(280.0f)
				.Justification(ETextJustify::Center)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 28))
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 8.0f)
		.HAlign(HAlign_Center)
		[
			SNew(SButton)
			.OnClicked(BIND_UOBJECT_DELEGATE(FOnClicked, HandleHowToPlaySlateClicked))
			.ContentPadding(FMargin(22.0f, 10.0f))
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("HOW TO PLAY")))
				.MinDesiredWidth(280.0f)
				.Justification(ETextJustify::Center)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 28))
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 8.0f)
		.HAlign(HAlign_Center)
		[
			SNew(SButton)
			.OnClicked(BIND_UOBJECT_DELEGATE(FOnClicked, HandleExitSlateClicked))
			.ContentPadding(FMargin(22.0f, 10.0f))
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("EXIT")))
				.MinDesiredWidth(280.0f)
				.Justification(ETextJustify::Center)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 28))
			]
		];
}

TSharedRef<SWidget> UMainMenuWidget::BuildNativeHowToPlayPage()
{
	const AMainMenuPlayerController* MenuController = GetOwningPlayer<AMainMenuPlayerController>();
	const FText NativeMissionText = MenuController ? MenuController->GetMissionText() : FText::GetEmpty();
	const TArray<FText> NativeControlLines = MenuController ? MenuController->GetControlLines() : TArray<FText>();

	TSharedRef<SVerticalBox> ControlsBox = SNew(SVerticalBox);

	for (const FText& ControlLine : NativeControlLines)
	{
		ControlsBox->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 4.0f)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(ControlLine)
				.Justification(ETextJustify::Center)
				.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f)))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 22))
			];
	}

	return SNew(SScrollBox)
		+ SScrollBox::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 80.0f, 0.0f, 28.0f)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("HOW TO PLAY")))
				.Justification(ETextJustify::Center)
				.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.84f, 0.18f, 1.0f)))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 48))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 8.0f)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("MISSION")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 28))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(260.0f, 0.0f, 260.0f, 26.0f)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(NativeMissionText)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.ColorAndOpacity(FSlateColor(FLinearColor(0.86f, 0.86f, 0.86f, 1.0f)))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 22))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 8.0f)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("CONTROLS")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 28))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 28.0f)
			.HAlign(HAlign_Center)
			[
				ControlsBox
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 8.0f)
			.HAlign(HAlign_Center)
			[
				SAssignNew(NativePlayButton, SButton)
				.OnClicked(BIND_UOBJECT_DELEGATE(FOnClicked, HandlePlaySlateClicked))
				.ContentPadding(FMargin(22.0f, 10.0f))
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("PLAY")))
					.MinDesiredWidth(280.0f)
					.Justification(ETextJustify::Center)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 28))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 8.0f, 0.0f, 80.0f)
			.HAlign(HAlign_Center)
			[
				SNew(SButton)
				.OnClicked(BIND_UOBJECT_DELEGATE(FOnClicked, HandleBackSlateClicked))
				.ContentPadding(FMargin(22.0f, 10.0f))
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("BACK")))
					.MinDesiredWidth(280.0f)
					.Justification(ETextJustify::Center)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 28))
				]
			]
		];
}

void UMainMenuWidget::PopulateMenuText()
{
	const AMainMenuPlayerController* MenuController = GetOwningPlayer<AMainMenuPlayerController>();

	if (!MenuController)
	{
		return;
	}

	if (MissionTextBlock)
	{
		MissionTextBlock->SetText(MenuController->GetMissionText());
	}

	PopulateControlLines(MenuController->GetControlLines());
}

void UMainMenuWidget::PopulateControlLines(const TArray<FText>& ControlLines)
{
	if (!ControlsContainer)
	{
		return;
	}

	ControlsContainer->ClearChildren();

	for (const FText& ControlLine : ControlLines)
	{
		UTextBlock* ControlTextBlock = NewObject<UTextBlock>(ControlsContainer);

		if (!ControlTextBlock)
		{
			continue;
		}

		ControlTextBlock->SetText(ControlLine);
		ControlTextBlock->SetAutoWrapText(false);
		ControlTextBlock->SetJustification(ETextJustify::Center);
		ControlTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f)));
		FSlateFontInfo FontInfo = ControlTextBlock->GetFont();
		FontInfo.Size = 22;
		ControlTextBlock->SetFont(FontInfo);

		UVerticalBoxSlot* ControlSlot = ControlsContainer->AddChildToVerticalBox(ControlTextBlock);

		if (ControlSlot)
		{
			ControlSlot->SetPadding(FMargin(0.0f, 4.0f));
			ControlSlot->SetHorizontalAlignment(HAlign_Fill);
		}
	}
}

UTextBlock* UMainMenuWidget::CreateTextBlock(const FText& Text, int32 FontSize, const FLinearColor& Color) const
{
	UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	TextBlock->SetText(Text);
	TextBlock->SetJustification(ETextJustify::Center);
	TextBlock->SetColorAndOpacity(FSlateColor(Color));

	FSlateFontInfo FontInfo = TextBlock->GetFont();
	FontInfo.Size = FontSize;
	TextBlock->SetFont(FontInfo);

	return TextBlock;
}

UButton* UMainMenuWidget::CreateMenuButton(const FText& Text) const
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	Button->SetBackgroundColor(FLinearColor(0.08f, 0.08f, 0.08f, 0.95f));

	UTextBlock* ButtonText = CreateTextBlock(Text, 28, FLinearColor::White);
	ButtonText->SetMinDesiredWidth(280.0f);
	ButtonText->SetMargin(FMargin(16.0f, 10.0f));
	Button->AddChild(ButtonText);

	return Button;
}

void UMainMenuWidget::AddTextToVerticalBox(UVerticalBox* TargetBox, UTextBlock* TextBlock, const FMargin& InPadding) const
{
	AddWidgetToVerticalBox(TargetBox, TextBlock, InPadding);
}

void UMainMenuWidget::AddWidgetToVerticalBox(UVerticalBox* TargetBox, UWidget* Widget, const FMargin& InPadding) const
{
	if (!TargetBox || !Widget)
	{
		return;
	}

	UVerticalBoxSlot* VerticalSlot = TargetBox->AddChildToVerticalBox(Widget);

	if (!VerticalSlot)
	{
		return;
	}

	VerticalSlot->SetPadding(InPadding);
	VerticalSlot->SetHorizontalAlignment(HAlign_Center);
}

FReply UMainMenuWidget::HandleStartSlateClicked()
{
	HandleStartClicked();
	return FReply::Handled();
}

FReply UMainMenuWidget::HandleHowToPlaySlateClicked()
{
	ShowHowToPlayPage();
	return FReply::Handled();
}

FReply UMainMenuWidget::HandleExitSlateClicked()
{
	HandleExitClicked();
	return FReply::Handled();
}

FReply UMainMenuWidget::HandlePlaySlateClicked()
{
	HandlePlayClicked();
	return FReply::Handled();
}

FReply UMainMenuWidget::HandleBackSlateClicked()
{
	ShowStartPage();
	return FReply::Handled();
}
