#include "ClockStatsWidget.h"

#include "ClockStatsSubsystem.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "AudioDevice.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UClockStatsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UBorder* Background = WidgetTree->ConstructWidget<UBorder>(
        UBorder::StaticClass()
    );
    Background->SetPadding(FMargin(14.0f, 10.0f));
    Background->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.02f, 0.78f));

    UVerticalBox* Menu = WidgetTree->ConstructWidget<UVerticalBox>(
        UVerticalBox::StaticClass()
    );

    StatsText = WidgetTree->ConstructWidget<UTextBlock>(
        UTextBlock::StaticClass()
    );
    StatsText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    StatsText->SetText(FText::FromString(TEXT("Score: 0\nItems: 0\nSize: --")));
    Menu->AddChild(StatsText);

    UHorizontalBox* Buttons = WidgetTree->ConstructWidget<UHorizontalBox>(
        UHorizontalBox::StaticClass()
    );
    Menu->AddChild(Buttons);

    auto AddButton = [this, Buttons](const FString& Label) -> UButton*
    {
        UButton* Button = WidgetTree->ConstructWidget<UButton>(
            UButton::StaticClass()
        );
        UTextBlock* ButtonText = WidgetTree->ConstructWidget<UTextBlock>(
            UTextBlock::StaticClass()
        );
        ButtonText->SetText(FText::FromString(Label));
        ButtonText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
        Button->SetContent(ButtonText);

        UHorizontalBoxSlot* ButtonSlot = Buttons->AddChildToHorizontalBox(Button);
        ButtonSlot->SetPadding(FMargin(3.0f, 8.0f, 3.0f, 0.0f));
        return Button;
    };

    UButton* MuteButton = AddButton(TEXT("MUTE"));
    MuteButtonText = Cast<UTextBlock>(MuteButton->GetContent());
    MuteButton->OnClicked.AddDynamic(this, &UClockStatsWidget::ToggleMute);

    UButton* RulesButton = AddButton(TEXT("RULES"));
    RulesButton->OnClicked.AddDynamic(this, &UClockStatsWidget::ToggleRules);

    UButton* RestartButton = AddButton(TEXT("START OVER"));
    RestartButton->OnClicked.AddDynamic(this, &UClockStatsWidget::StartOver);

    UButton* ExitButton = AddButton(TEXT("EXIT"));
    ExitButton->OnClicked.AddDynamic(this, &UClockStatsWidget::ExitGame);

    RulesText = WidgetTree->ConstructWidget<UTextBlock>(
        UTextBlock::StaticClass()
    );
    RulesText->SetColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f)));
    RulesText->SetAutoWrapText(true);
    RulesText->SetText(FText::FromString(
        TEXT("RULES\n"
             "Eat objects smaller than the clock to grow.\n"
             "Lanterns are worth extra points.\n"
             "Avoid objects that are too large.\n"
             "Use START OVER to restart the level.")
    ));
    RulesText->SetVisibility(ESlateVisibility::Collapsed);
    Menu->AddChild(RulesText);

    Background->SetContent(Menu);
    WidgetTree->RootWidget = Background;
    RefreshStats();
}

void UClockStatsWidget::NativeTick(
    const FGeometry& MyGeometry,
    float InDeltaTime
)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    RefreshStats();
}

void UClockStatsWidget::RefreshStats()
{
    if (!IsValid(StatsText) || !GetWorld())
    {
        return;
    }

    const UClockStatsSubsystem* Stats =
        GetWorld()->GetSubsystem<UClockStatsSubsystem>();

    if (!IsValid(Stats))
    {
        return;
    }

    const FString SizeText = Stats->GetCurrentRadius() > 0.0f
        ? FString::Printf(TEXT("%.1f"), Stats->GetCurrentRadius())
        : TEXT("--");

    StatsText->SetText(FText::FromString(FString::Printf(
        TEXT("SCORE  %d\nITEMS  %d\nSIZE  %s"),
        Stats->GetScore(),
        Stats->GetItemsConsumed(),
        *SizeText
    )));
}

void UClockStatsWidget::ToggleMute()
{
    bAudioMuted = !bAudioMuted;

    if (GEngine && GEngine->GetMainAudioDevice())
    {
        GEngine->GetMainAudioDevice()->SetTransientPrimaryVolume(
            bAudioMuted ? 0.0f : 1.0f
        );
    }

    if (IsValid(MuteButtonText))
    {
        MuteButtonText->SetText(FText::FromString(
            bAudioMuted ? TEXT("UNMUTE") : TEXT("MUTE")
        ));
    }
}

void UClockStatsWidget::ToggleRules()
{
    if (!IsValid(RulesText))
    {
        return;
    }

    const bool bRulesVisible =
        RulesText->GetVisibility() == ESlateVisibility::Visible;
    RulesText->SetVisibility(
        bRulesVisible
            ? ESlateVisibility::Collapsed
            : ESlateVisibility::Visible
    );
}

void UClockStatsWidget::StartOver()
{
    if (UWorld* World = GetWorld())
    {
        UGameplayStatics::OpenLevel(
            World,
            FName(*World->GetName())
        );
    }
}

void UClockStatsWidget::ExitGame()
{
    if (APlayerController* PlayerController = GetOwningPlayer())
    {
        UKismetSystemLibrary::QuitGame(
            this,
            PlayerController,
            EQuitPreference::Quit,
            false
        );
    }
}
