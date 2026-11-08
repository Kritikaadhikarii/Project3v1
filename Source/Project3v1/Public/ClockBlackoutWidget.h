#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ClockBlackoutWidget.generated.h"

class UTextBlock;

UCLASS()
class PROJECT3V1_API UClockBlackoutWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void StartCountdown(float Duration);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    UPROPERTY()
    TObjectPtr<UTextBlock> MessageText;

    float CountdownEndTime = 0.0f;
};
