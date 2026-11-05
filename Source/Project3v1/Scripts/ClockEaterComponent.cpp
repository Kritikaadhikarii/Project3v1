
#include "ClockEaterComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "CollisionShape.h"
#include "GameFramework/Actor.h"

UClockEaterComponent::UClockEaterComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UClockEaterComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (!Owner) return;

    TArray<UStaticMeshComponent*> Meshes;
    Owner->GetComponents<UStaticMeshComponent>(Meshes);

    for (UStaticMeshComponent* Mesh : Meshes)
    {
        if (Mesh && Mesh->ComponentHasTag(TEXT("ClockVisual")))
        {
            ClockVisual = Mesh;
            break;
        }
    }

    if (!ClockVisual)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("ClockVisual mesh tag not found!"));
    }
}

void UClockEaterComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(
        DeltaTime, TickType, ThisTickFunction);

    if (CurrentItem.IsValid())
    {
        UpdateSwallow(DeltaTime);
        return;
    }

    SearchTimer += DeltaTime;

    if (SearchTimer >= 0.15f)
    {
        SearchTimer = 0.f;
        FindEdible();
    }
}

void UClockEaterComponent::FindEdible()
{
    AActor* Owner = GetOwner();
    UWorld* World = GetWorld();

    if (!Owner || !World) return;

    TArray<FOverlapResult> Results;

    FCollisionObjectQueryParams ObjectTypes;
    ObjectTypes.AddObjectTypesToQuery(ECC_WorldDynamic);
    ObjectTypes.AddObjectTypesToQuery(ECC_PhysicsBody);

    World->OverlapMultiByObjectType(
        Results,
        Owner->GetActorLocation(),
        FQuat::Identity,
        ObjectTypes,
        FCollisionShape::MakeSphere(DetectionRadius)
    );

    for (const FOverlapResult& Result : Results)
    {
        AActor* Item = Result.GetActor();

        if (!IsValid(Item) || Item == Owner)
            continue;

        if (!Item->ActorHasTag(TEXT("Edible")))
            continue;

        FVector Origin, Extent;
        Item->GetActorBounds(false, Origin, Extent);

        // Use the largest dimension as the object's size.
        float ItemSize = 2.f * Extent.GetMax();

        if (ItemSize > MaxEdibleSize)
            continue;

        StartSwallow(Item);
        break;
    }
}

void UClockEaterComponent::StartSwallow(AActor* Item)
{
    if (!IsValid(Item)) return;

    CurrentItem = Item;

    ItemStartLocation = Item->GetActorLocation();
    ItemStartScale = Item->GetActorScale3D();

    SwallowTime = 0.f;

    // Prevent the shrinking object from blocking movement.
    Item->SetActorEnableCollision(false);

    UE_LOG(LogTemp, Display,
        TEXT("Swallow started: %s"), *Item->GetName());
}

void UClockEaterComponent::UpdateSwallow(float DeltaTime)
{
    if (!CurrentItem.IsValid())
        return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    SwallowTime += DeltaTime;

    float Alpha = FMath::Clamp(
        SwallowTime / FMath::Max(SwallowDuration, 0.01f),
        0.f,
        1.f
    );

    // Smooth animation.
    float SmoothAlpha = FMath::InterpEaseInOut(
        0.f, 1.f, Alpha, 2.f
    );

    FVector TargetLocation =
        Owner->GetActorLocation() + FVector(0, 0, -35.f);

    FVector NewLocation = FMath::Lerp(
        ItemStartLocation,
        TargetLocation,
        SmoothAlpha
    );

    FVector NewScale = FMath::Lerp(
        ItemStartScale,
        ItemStartScale * 0.03f,
        SmoothAlpha
    );

    CurrentItem->SetActorLocation(NewLocation);
    CurrentItem->SetActorScale3D(NewScale);

    if (Alpha >= 1.f)
    {
        FinishSwallow();
    }
}

void UClockEaterComponent::FinishSwallow()
{
    if (!CurrentItem.IsValid())
        return;

    CurrentItem->Destroy();
    CurrentItem.Reset();

    // Increase the size of objects the clock can eat.
    MaxEdibleSize *= GrowthMultiplier;

    // Increase detection range slightly.
    DetectionRadius *= 1.05f;

    // Grow only the clock's visual mesh,
    // not the entire player character.
    if (ClockVisual)
    {
        ClockVisual->SetRelativeScale3D(
            ClockVisual->GetRelativeScale3D()
            * GrowthMultiplier
        );
    }

    Score += 10;

    UE_LOG(LogTemp, Display,
        TEXT("Object swallowed! Score: %d | Max size: %.1f"),
        Score, MaxEdibleSize);
}
