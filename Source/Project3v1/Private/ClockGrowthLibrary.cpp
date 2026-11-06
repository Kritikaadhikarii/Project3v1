#include "ClockGrowthLibrary.h"
#include "Components/SceneComponent.h"

float UClockGrowthLibrary::GrowClockMesh(USceneComponent* ClockMesh, float CurrentRadius, float GrowthAmount)
{
    const float OldRadius = FMath::Max(CurrentRadius, 0.01f);
    const float AddedRadius = FMath::Max(GrowthAmount, 0.0f);
    const float NewRadius = OldRadius + AddedRadius;

    // Scale proportionally so visible growth tracks the gameplay EatingRadius.
    // This assumes the initial EatingRadius corresponds to the visible clock mesh.
    if (IsValid(ClockMesh) && AddedRadius > 0.0f)
    {
        const float ScaleFactor = NewRadius / OldRadius;
        ClockMesh->SetWorldScale3D(ClockMesh->GetComponentScale() * ScaleFactor);
    }
    return NewRadius;
}
