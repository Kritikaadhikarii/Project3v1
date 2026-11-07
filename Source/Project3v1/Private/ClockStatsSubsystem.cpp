#include "ClockStatsSubsystem.h"

void UClockStatsSubsystem::RecordReward(int32 PointsEarned)
{
    Score += FMath::Max(0, PointsEarned);
}

void UClockStatsSubsystem::RecordSwallow(float NewRadius)
{
    ++ItemsConsumed;
    CurrentRadius = FMath::Max(0.0f, NewRadius);
}
