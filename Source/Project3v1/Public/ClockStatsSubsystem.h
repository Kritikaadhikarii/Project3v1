#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ClockStatsSubsystem.generated.h"

UCLASS()
class PROJECT3V1_API UClockStatsSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    void RecordReward(int32 PointsEarned);
    void RecordSwallow(float NewRadius);
    void RecordWin() { bHasWon = true; }

    int32 GetScore() const { return Score; }
    int32 GetItemsConsumed() const { return ItemsConsumed; }
    float GetCurrentRadius() const { return CurrentRadius; }
    bool HasWon() const { return bHasWon; }

private:
    int32 Score = 0;
    int32 ItemsConsumed = 0;
    float CurrentRadius = 0.0f;
    bool bHasWon = false;
};
