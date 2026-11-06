
#include "ClockGrowth.h"

#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

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

    // Grow the visible clock mesh.
    TArray<UStaticMeshComponent*> Meshes;
    ClockActor->GetComponents<UStaticMeshComponent>(Meshes);

    bool bMeshFound = false;

    for (UStaticMeshComponent* Mesh : Meshes)
    {
        if (IsValid(Mesh) &&
            Mesh->GetName().Contains(TEXT("ClockMesh")))
        {
            if (CurrentRadius > 0.0f)
            {
                const float Factor =
                    NewRadius / CurrentRadius;

                Mesh->SetRelativeScale3D(
                    Mesh->GetRelativeScale3D() * Factor
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

    // Expand the overlap sphere if EatingZone is a sphere.
    TArray<USphereComponent*> Spheres;
    ClockActor->GetComponents<USphereComponent>(Spheres);

    for (USphereComponent* Sphere : Spheres)
    {
        if (IsValid(Sphere) &&
            Sphere->GetName().Contains(TEXT("EatingZone")))
        {
            const float Factor =
                CurrentRadius > 0.0f
                ? NewRadius / CurrentRadius
                : 1.0f;

            Sphere->SetSphereRadius(
                Sphere->GetUnscaledSphereRadius() * Factor,
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
