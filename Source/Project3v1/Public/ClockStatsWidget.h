#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/SWidget.h"
#include "ClockStatsWidget.generated.h"

class UTextBlock;
class UButton;
class UBorder;
class UVerticalBox;
class UAudioComponent;
class UCanvasPanelSlot;

UCLASS()
class PROJECT3V1_API UClockStatsWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
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
    void RestartGame();

    UFUNCTION()
    void ExitGame();

    UFUNCTION()
    void AddEdibles();

    UPROPERTY()
    TObjectPtr<UTextBlock> StatsText;

    UPROPERTY()
    TObjectPtr<UTextBlock> MuteButtonText;

    UPROPERTY()
    TObjectPtr<UTextBlock> RulesText;

    UPROPERTY()
    TObjectPtr<UButton> AddEdiblesButton;

    UPROPERTY()
    TObjectPtr<UVerticalBox> WinPanel;

    UPROPERTY()
    TObjectPtr<UBorder> WinOverlay;

    UPROPERTY()
    TObjectPtr<UCanvasPanelSlot> MainPanelSlot;

    TMap<TObjectPtr<UAudioComponent>, float> PlayerAudioVolumes;
    bool bAudioMuted = false;
    bool bGamePausedForWin = false;
};
