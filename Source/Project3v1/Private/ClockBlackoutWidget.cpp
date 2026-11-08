#include "ClockBlackoutWidget.h"

#include "Components/TextBlock.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/World.h"

void UClockBlackoutWidget::NativeConstruct()
{
    Super::NativeConstruct();

    MessageText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    MessageText->SetJustification(ETextJustify::Center);
    MessageText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    MessageText->SetText(FText::FromString(TEXT("BLACKOUT\nYou can still move\n5")));
    WidgetTree->RootWidget = MessageText;
}

void UClockBlackoutWidget::StartCountdown(float Duration)
{
    if (UWorld* World = GetWorld())
    {
        CountdownEndTime = World->GetTimeSeconds() + Duration;
    }

    SetVisibility(ESlateVisibility::Visible);
}

void UClockBlackoutWidget::NativeTick(
    const FGeometry& MyGeometry,
    float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!GetWorld() || !MessageText)
    {
        return;
    }

    const float Remaining = CountdownEndTime - GetWorld()->GetTimeSeconds();
    const int32 Seconds = FMath::Max(0, FMath::CeilToInt(Remaining));

    if (Seconds <= 0)
    {
        SetVisibility(ESlateVisibility::Collapsed);
        return;
    }

    MessageText->SetText(FText::FromString(FString::Printf(
        TEXT("BLACKOUT\nYou can still move\n%d"),
        Seconds
    )));
}
