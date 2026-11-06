
#include "ClockGrowth.h"

#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/PrimitiveComponent.h"

// ==================================================
// FUNCTION 1: Grow the clock after swallowing
// ==================================================

float UClockGrowth::FinishEating(
    AActor* ClockActor,
    float CurrentRadius,
    float GrowthAmount)
{
    if (!IsValid(ClockActor))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("Growth failed: ClockActor invalid"));
        return CurrentRadius;
    }

    const float SafeGrowth =
        FMath::Max(0.0f, GrowthAmount);

    const float NewRadius =
        CurrentRadius + SafeGrowth;

    // Find and grow the visible ClockMesh.
    TArray<UStaticMeshComponent*> Meshes;

    ClockActor->GetComponents<UStaticMeshComponent>(
        Meshes
    );

    bool bMeshFound = false;

    for (UStaticMeshComponent* Mesh : Meshes)
    {
        if (IsValid(Mesh) &&
            Mesh->GetName().Contains(TEXT("ClockMesh")))
        {
            if (CurrentRadius > 0.0f)
            {
                const float ScaleFactor =
                    NewRadius / CurrentRadius;

                Mesh->SetRelativeScale3D(
                    Mesh->GetRelativeScale3D()
                    * ScaleFactor
                );

                bMeshFound = true;
            }

            break;
        }
    }

    if (!bMeshFound)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("ClockMesh not found or radius invalid"));
    }

    // Expand EatingZone if it is a sphere.
    TArray<USphereComponent*> Spheres;

    ClockActor->GetComponents<USphereComponent>(
        Spheres
    );

    for (USphereComponent* Sphere : Spheres)
    {
        if (IsValid(Sphere) &&
            Sphere->GetName().Contains(TEXT("EatingZone")))
        {
            const float ScaleFactor =
                CurrentRadius > 0.0f
                ? NewRadius / CurrentRadius
                : 1.0f;

            Sphere->SetSphereRadius(
                Sphere->GetUnscaledSphereRadius()
                * ScaleFactor,
                true
            );

            break;
        }
    }

    UE_LOG(LogTemp, Warning,
        TEXT("Swallowed! Clock grew: %.2f -> %.2f"),
        CurrentRadius,
        NewRadius
    );

    return NewRadius;
}


// ==================================================
// FUNCTION 2: Calculate dynamic points and growth
// ==================================================

void UClockGrowth::CalculateEatReward(
    UPrimitiveComponent* ObjectMesh,
    int32& PointsEarned,
    float& GrowthAmount)
{
    PointsEarned = 0;
    GrowthAmount = 0.0f;

    if (!IsValid(ObjectMesh))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("Reward failed: Invalid ObjectMesh"));
        return;
    }

    // Get world-space mesh bounding dimensions.
    // Bounds.BoxExtent contains half the dimensions.
    const FBoxSphereBounds Bounds =
        ObjectMesh->Bounds;

    const FVector Extent =
        Bounds.BoxExtent;

    // Calculate the object's footprint.
    const float Width =
        Extent.X * 2.0f;

    const float Depth =
        Extent.Y * 2.0f;

    const float Area =
        Width * Depth;

    // Points scale with the object's footprint.
    PointsEarned = FMath::Max(
        1,
        FMath::RoundToInt(Area / 1000.0f)
    );

    // Larger objects give more growth.
    // No fixed maximum growth cap.
    GrowthAmount = FMath::Max(
        0.5f,
        FMath::Sqrt(Area) * 0.025f
    );

    UE_LOG(LogTemp, Warning,
        TEXT("Reward! Area: %.1f | Points: %d | Growth: %.2f"),
        Area,
        PointsEarned,
        GrowthAmount
    );
}
