
#include "ClockGrowth.h"

#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMesh.h"
#include "ClockBlackoutSubsystem.h"
#include "ClockStatsSubsystem.h"

// ==================================================
// HELPER: Identify a lantern
// ==================================================

namespace
{
    bool IsLanternName(const FString& Name)
    {
        return Name.Contains(
            TEXT("lantern"),
            ESearchCase::IgnoreCase
        ) ||
            Name.Contains(
                TEXT("latern"),
                ESearchCase::IgnoreCase
            );
    }

    bool IsLanternObject(UPrimitiveComponent* ObjectMesh)
    {
        if (!IsValid(ObjectMesh))
        {
            return false;
        }

        // 1. Check component name.
        if (IsLanternName(ObjectMesh->GetName()))
        {
            return true;
        }

        // 2. Check imported static mesh asset name.
        if (UStaticMeshComponent* StaticMesh =
            Cast<UStaticMeshComponent>(ObjectMesh))
        {
            if (UStaticMesh* MeshAsset =
                StaticMesh->GetStaticMesh())
            {
                if (IsLanternName(MeshAsset->GetName()))
                {
                    return true;
                }
            }
        }

        // 3. Check owning actor name.
        if (AActor* Owner = ObjectMesh->GetOwner())
        {
            if (IsLanternName(Owner->GetName()))
            {
                return true;
            }

            // 4. Check other meshes on the actor.
            TArray<UStaticMeshComponent*> Meshes;

            Owner->GetComponents<UStaticMeshComponent>(
                Meshes
            );

            for (UStaticMeshComponent* Mesh : Meshes)
            {
                if (!IsValid(Mesh))
                {
                    continue;
                }

                if (IsLanternName(Mesh->GetName()))
                {
                    return true;
                }

                if (UStaticMesh* Asset =
                    Mesh->GetStaticMesh())
                {
                    if (IsLanternName(Asset->GetName()))
                    {
                        return true;
                    }
                }
            }
        }

        return false;
    }
}


// ==================================================
// FUNCTION 1: Grow clock after swallowing
// ==================================================

float UClockGrowth::FinishEating(
    AActor* ClockActor,
    float CurrentRadius,
    float GrowthAmount)
{
    if (!IsValid(ClockActor))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Growth failed: ClockActor invalid")
        );

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
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("ClockMesh not found or radius invalid")
        );
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

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Swallowed! Clock grew: %.2f -> %.2f"),
        CurrentRadius,
        NewRadius
    );

    if (UWorld* World = ClockActor->GetWorld())
    {
        if (UClockStatsSubsystem* Stats =
            World->GetSubsystem<UClockStatsSubsystem>())
        {
            Stats->RecordSwallow(NewRadius);
        }
    }

    return NewRadius;
}

float UClockGrowth::SetRadius(
    AActor* ClockActor,
    float CurrentRadius,
    float TargetRadius)
{
    if (!IsValid(ClockActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("Resize failed: ClockActor invalid"));
        return CurrentRadius;
    }

    const float SafeCurrent = FMath::Max(CurrentRadius, 0.01f);
    const float NewRadius = FMath::Max(TargetRadius, 0.01f);
    const float ScaleFactor = NewRadius / SafeCurrent;

    TArray<UStaticMeshComponent*> Meshes;
    ClockActor->GetComponents<UStaticMeshComponent>(Meshes);
    for (UStaticMeshComponent* Mesh : Meshes)
    {
        if (IsValid(Mesh) &&
            Mesh->GetName().Contains(TEXT("ClockMesh")))
        {
            Mesh->SetRelativeScale3D(
                Mesh->GetRelativeScale3D() * ScaleFactor);
            break;
        }
    }

    TArray<USphereComponent*> Spheres;
    ClockActor->GetComponents<USphereComponent>(Spheres);
    for (USphereComponent* Sphere : Spheres)
    {
        if (IsValid(Sphere) &&
            Sphere->GetName().Contains(TEXT("EatingZone")))
        {
            Sphere->SetSphereRadius(
                Sphere->GetUnscaledSphereRadius() * ScaleFactor, true);
            break;
        }
    }

    if (UWorld* World = ClockActor->GetWorld())
    {
        if (UClockStatsSubsystem* Stats =
            World->GetSubsystem<UClockStatsSubsystem>())
        {
            Stats->RecordSwallow(NewRadius);
        }
    }

    return NewRadius;
}


// ==================================================
// FUNCTION 2: Calculate points and growth
//
// NORMAL OBJECT:
//     Normal points
//     Normal growth
//
// LANTERN:
//     DOUBLE points
//     Normal growth
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
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Reward failed: Invalid ObjectMesh")
        );

        return;
    }

    // Calculate original horizontal footprint.
    const FBoxSphereBounds LocalBounds =
        ObjectMesh->CalcBounds(
            FTransform::Identity
        );

    const FVector LocalSize =
        LocalBounds.BoxExtent * 2.0f;

    // Read object scale before swallowing animation.
    const FVector ObjectScale =
        ObjectMesh->GetComponentScale();

    const float Width =
        LocalSize.X * FMath::Abs(ObjectScale.X);

    const float Depth =
        LocalSize.Y * FMath::Abs(ObjectScale.Y);

    const float Area =
        FMath::Max(0.0f, Width * Depth);

    // --------------------------------------------
    // Normal points
    // --------------------------------------------

    PointsEarned = FMath::Max(
        1,
        FMath::RoundToInt(Area / 1000.0f)
    );

    // --------------------------------------------
    // Normal growth
    // --------------------------------------------

    GrowthAmount = FMath::Max(
        2.0f,
        FMath::Sqrt(Area) * 0.15f
    );

    // --------------------------------------------
    // Special lantern reward
    // --------------------------------------------

    const bool bIsLantern =
        IsLanternObject(ObjectMesh);

    if (bIsLantern)
    {
        PointsEarned *= 2;

        if (UWorld* World = ObjectMesh->GetWorld())
        {
            if (UClockBlackoutSubsystem* Blackout =
                World->GetSubsystem<UClockBlackoutSubsystem>())
            {
                Blackout->TriggerBlackout();
            }
        }

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("SPECIAL LANTERN SWALLOWED! DOUBLE POINTS!")
        );
    }

    // --------------------------------------------
    // Final reward log
    // --------------------------------------------

    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "Reward! Lantern: %s | Area: %.1f | Points: %d | Growth: %.2f"
        ),
        bIsLantern ? TEXT("YES") : TEXT("NO"),
        Area,
        PointsEarned,
        GrowthAmount
    );

    if (UWorld* World = ObjectMesh->GetWorld())
    {
        if (UClockStatsSubsystem* Stats =
            World->GetSubsystem<UClockStatsSubsystem>())
        {
            Stats->RecordReward(PointsEarned);
        }
    }
}
