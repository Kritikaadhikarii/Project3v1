#include "ClockStatsWidget.h"

#include "ClockStatsSubsystem.h"
#include "Scripts/ClockEaterComponent.h"
#include "Project3v1Character.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/AudioComponent.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "AudioDevice.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

TSharedRef<SWidget> UClockStatsWidget::RebuildWidget()
{
    if (WidgetTree && WidgetTree->RootWidget)
    {
        return Super::RebuildWidget();
    }

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
    StatsText->SetText(FText::FromString(TEXT("GAME STATS\nSCORE  0\nITEMS  0\nSIZE  --")));
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

    AddEdiblesButton = AddButton(TEXT("ADD EDIBLES"));
    AddEdiblesButton->OnClicked.AddDynamic(this, &UClockStatsWidget::AddEdibles);

    UButton* ExitButton = AddButton(TEXT("EXIT"));
    ExitButton->OnClicked.AddDynamic(this, &UClockStatsWidget::ExitGame);

    RulesText = WidgetTree->ConstructWidget<UTextBlock>(
        UTextBlock::StaticClass()
    );
    RulesText->SetColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f)));
    RulesText->SetAutoWrapText(true);
    RulesText->SetText(FText::FromString(
        TEXT("HOW TO PLAY\n"
             "Move: W/A/S/D or the left stick.\n"
             "Look: mouse or the right stick.\n"
             "Jump: Space or the controller jump button.\n"
             "\n"
             "GROW YOUR CLOCK\n"
             "Find edible objects and move close to them.\n"
             "The clock automatically swallows nearby objects that fit inside "
             "its current size.\n"
             "You cannot swallow an object that is too large. Keep eating "
             "smaller objects until you grow.\n"
             "Your SIZE shows the current swallowing radius.\n"
             "\n"
             "SPECIAL OBJECTS\n"
             "Lanterns give double points and trigger a 6-second blackout. "
             "You can still move during a blackout.\n"
             "Cacti and bushes are dangerous: swallowing one shrinks your "
             "clock to 75% of its previous size and scrambles movement for "
             "10 seconds.\n"
             "\n"
             "WIN CONDITION\n"
             "Grow large enough to swallow the final building. The building's "
             "entire ground footprint must fit inside your clock.\n"
             "After the building is swallowed, the game pauses and the "
             "GAME WON message appears.\n"
             "\n"
             "MENU BUTTONS\n"
              "MUTE toggles audio. ADD EDIBLES spawns extra food only once "
              "per game. "
             "START OVER restarts the level. EXIT closes the game.")
    ));
    RulesText->SetVisibility(ESlateVisibility::Collapsed);
    Menu->AddChild(RulesText);

    WinPanel = WidgetTree->ConstructWidget<UVerticalBox>(
        UVerticalBox::StaticClass()
    );
    WinPanel->SetVisibility(ESlateVisibility::Collapsed);

    UTextBlock* WinText = WidgetTree->ConstructWidget<UTextBlock>(
        UTextBlock::StaticClass()
    );
    WinText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.85f, 0.2f)));
    WinText->SetJustification(ETextJustify::Center);
    WinText->SetText(FText::FromString(
        TEXT("YOU WIN!\nThe final building was swallowed.")
    ));
    if (UVerticalBoxSlot* WinTextSlot =
        Cast<UVerticalBoxSlot>(WinPanel->AddChild(WinText)))
    {
        WinTextSlot->SetHorizontalAlignment(HAlign_Center);
    }

    UHorizontalBox* WinButtons = WidgetTree->ConstructWidget<UHorizontalBox>(
        UHorizontalBox::StaticClass()
    );
    if (UVerticalBoxSlot* WinButtonsSlot =
        Cast<UVerticalBoxSlot>(WinPanel->AddChild(WinButtons)))
    {
        WinButtonsSlot->SetHorizontalAlignment(HAlign_Center);
    }

    auto AddWinButton = [this, WinButtons](const FString& Label) -> UButton*
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

        UHorizontalBoxSlot* ButtonSlot =
            WinButtons->AddChildToHorizontalBox(Button);
        ButtonSlot->SetPadding(FMargin(3.0f, 8.0f, 3.0f, 0.0f));
        return Button;
    };

    UButton* PlayAgainButton = AddWinButton(TEXT("PLAY AGAIN"));
    PlayAgainButton->OnClicked.AddDynamic(
        this, &UClockStatsWidget::StartOver);

    UButton* WinRestartButton = AddWinButton(TEXT("RESTART"));
    WinRestartButton->OnClicked.AddDynamic(
        this, &UClockStatsWidget::RestartGame);

    Menu->AddChild(WinPanel);

    Background->SetContent(Menu);
    USizeBox* Panel = WidgetTree->ConstructWidget<USizeBox>(
        USizeBox::StaticClass()
    );
    Panel->SetMinDesiredWidth(620.0f);
    Panel->SetMinDesiredHeight(560.0f);
    Panel->SetContent(Background);

    UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(
        UCanvasPanel::StaticClass()
    );
    MainPanelSlot = RootCanvas->AddChildToCanvas(Panel);
    MainPanelSlot->SetPosition(FVector2D(20.0f, 20.0f));
    MainPanelSlot->SetSize(FVector2D(620.0f, 560.0f));

    WinOverlay = WidgetTree->ConstructWidget<UBorder>(
        UBorder::StaticClass()
    );
    WinOverlay->SetBrushColor(FLinearColor(0.02f, 0.25f, 0.85f, 0.94f));
    WinOverlay->SetVisibility(ESlateVisibility::Collapsed);
    UCanvasPanelSlot* OverlaySlot = RootCanvas->AddChildToCanvas(WinOverlay);
    OverlaySlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
    OverlaySlot->SetOffsets(FMargin(0.0f));
    OverlaySlot->SetZOrder(-1);

    WidgetTree->RootWidget = RootCanvas;
    return Super::RebuildWidget();
}

void UClockStatsWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetVisibility(ESlateVisibility::Visible);
    SetRenderOpacity(1.0f);
    SetIsEnabled(true);
    if (APlayerController* PlayerController = GetOwningPlayer())
    {
        PlayerController->bShowMouseCursor = true;
        PlayerController->SetInputMode(FInputModeGameAndUI());
    }
    RefreshStats();

    UE_LOG(LogTemp, Display, TEXT("Clock stats widget constructed with buttons and game stats"));
}

void UClockStatsWidget::NativeDestruct()
{
    if (bAudioMuted)
    {
        ToggleMute();
    }

    Super::NativeDestruct();
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

    const FString StatusText = Stats->HasWon()
        ? TEXT("\nBUILDING SWALLOWED - YOU WIN!")
        : TEXT("\nGrow until you can swallow the big building.");
    StatsText->SetText(FText::FromString(FString::Printf(
        TEXT("GAME STATS\nSCORE  %d\nITEMS  %d\nSIZE  %s%s"),
        Stats->GetScore(),
        Stats->GetItemsConsumed(),
        *SizeText,
        *StatusText
    )));

    WinPanel->SetVisibility(
        Stats->HasWon()
            ? ESlateVisibility::Visible
            : ESlateVisibility::Collapsed
    );

    if (IsValid(WinOverlay))
    {
        const bool bHasWon = Stats->HasWon();
        WinOverlay->SetVisibility(
            bHasWon
                ? ESlateVisibility::Visible
                : ESlateVisibility::Collapsed
        );

        if (bHasWon && !bGamePausedForWin)
        {
            bGamePausedForWin = true;
            UGameplayStatics::SetGamePaused(GetWorld(), true);
        }
    }
}

void UClockStatsWidget::AddEdibles()
{
    if (!GetWorld() || !IsValid(AddEdiblesButton) ||
        !AddEdiblesButton->GetIsEnabled())
    {
        return;
    }

    APlayerController* PlayerController = GetOwningPlayer();
    AProject3v1Character* Character = PlayerController
        ? Cast<AProject3v1Character>(PlayerController->GetPawn())
        : nullptr;
    if (!Character || !IsValid(Character->ClockEater))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot add edibles: clock eater is unavailable"));
        return;
    }

    if (Character->ClockEater->AddEdibles())
    {
        AddEdiblesButton->SetIsEnabled(false);
    }
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

    if (APlayerController* PlayerController = GetOwningPlayer())
    {
        APawn* PlayerPawn = PlayerController->GetPawn();
        if (bAudioMuted)
        {
            PlayerAudioVolumes.Empty();

            if (IsValid(PlayerPawn))
            {
                TArray<UAudioComponent*> AudioComponents;
                PlayerPawn->GetComponents<UAudioComponent>(AudioComponents);
                for (UAudioComponent* AudioComponent : AudioComponents)
                {
                    if (!IsValid(AudioComponent))
                    {
                        continue;
                    }

                    PlayerAudioVolumes.Add(
                        AudioComponent,
                        AudioComponent->VolumeMultiplier);
                    AudioComponent->SetVolumeMultiplier(0.0f);
                }
            }
        }
        else
        {
            for (const TPair<TObjectPtr<UAudioComponent>, float>& Entry :
                PlayerAudioVolumes)
            {
                if (IsValid(Entry.Key))
                {
                    Entry.Key->SetVolumeMultiplier(Entry.Value);
                }
            }
            PlayerAudioVolumes.Empty();
        }
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
    if (!IsValid(RulesText) || !IsValid(MainPanelSlot))
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

    if (bRulesVisible)
    {
        MainPanelSlot->SetAnchors(FAnchors(0.0f, 0.0f));
        MainPanelSlot->SetOffsets(FMargin(20.0f));
        MainPanelSlot->SetPosition(FVector2D(20.0f, 20.0f));
        MainPanelSlot->SetSize(FVector2D(620.0f, 560.0f));
    }
    else
    {
        MainPanelSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
        MainPanelSlot->SetOffsets(FMargin(0.0f));
    }
}

void UClockStatsWidget::StartOver()
{
    if (UWorld* World = GetWorld())
    {
        UGameplayStatics::SetGamePaused(World, false);
        UGameplayStatics::OpenLevel(
            World,
            FName(*World->GetName())
        );
    }
}

void UClockStatsWidget::RestartGame()
{
    StartOver();
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
