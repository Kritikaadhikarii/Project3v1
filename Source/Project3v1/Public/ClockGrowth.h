
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ClockGrowth.generated.h"

UCLASS()
class PROJECT3V1_API UClockGrowth
    : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category = "Clock Eater")
    static float FinishEating(
        AActor* ClockActor,
        float CurrentRadius,
        float GrowthAmount = 5.0f
    );
};
