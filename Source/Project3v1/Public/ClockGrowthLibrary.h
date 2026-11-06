#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ClockGrowthLibrary.generated.h"

class USceneComponent;

UCLASS()
class UClockGrowthLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Updates a clock mesh's visible scale and returns the new eating radius.
     * The GrowthAmount and CurrentRadius are both in Unreal world units.
     * This function should run ONCE after a successful swallow.
     */
    UFUNCTION(BlueprintCallable, Category="Clock Eater|Growth", meta=(DisplayName="Grow Clock Mesh"))
    static float GrowClockMesh(USceneComponent* ClockMesh, float CurrentRadius, float GrowthAmount);
};
