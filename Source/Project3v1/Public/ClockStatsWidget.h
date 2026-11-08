#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/SWidget.h"
#include "ClockStatsWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class PROJECT3V1_API UClockStatsWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(
        const FGeometry& MyGeometry,
        float InDeltaTime
    ) override;

private:
    void RefreshStats();

    UFUNCTION()
    void ToggleMute();

    UFUNCTION()
    void ToggleRules();

    UFUNCTION()
    void StartOver();

    UFUNCTION()
    void ExitGame();

    UPROPERTY()
    TObjectPtr<UTextBlock> StatsText;

    UPROPERTY()
    TObjectPtr<UTextBlock> MuteButtonText;

    UPROPERTY()
    TObjectPtr<UTextBlock> RulesText;

    bool bAudioMuted = false;
};
