
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ClockGrowth.generated.h"

class UPrimitiveComponent;

UCLASS()
class PROJECT3V1_API UClockGrowth
    : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    // Grow the clock after swallowing.
    UFUNCTION(BlueprintCallable, Category = "Clock Eater")
    static float FinishEating(
        AActor* ClockActor,
        float CurrentRadius,
        float GrowthAmount = 5.0f
    );

    // Calculate rewards based on object footprint.
    UFUNCTION(BlueprintCallable, Category = "Clock Eater")
    static void CalculateEatReward(
        UPrimitiveComponent* ObjectMesh,
        int32& PointsEarned,
        float& GrowthAmount
    );
};
