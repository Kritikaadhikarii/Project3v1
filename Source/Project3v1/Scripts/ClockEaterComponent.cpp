
#include "ClockEaterComponent.h"

#include "ClockGrowth.h"
#include "ClockStatsSubsystem.h"
#include "ClockPawn.h"
#include "Project3v1Character.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Engine/OverlapResult.h"
#include "EngineUtils.h"
#include "CollisionShape.h"
#include "WorldCollision.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "UObject/UnrealType.h"

namespace
{
    bool IsFinalBuildingName(const FString& Name)
    {
        FString NormalizedName = Name;
        NormalizedName.ReplaceInline(TEXT(" "), TEXT(""));
        NormalizedName.ReplaceInline(TEXT("_"), TEXT(""));
        NormalizedName.ReplaceInline(TEXT("-"), TEXT(""));
        return NormalizedName.Contains(TEXT("finalbuilding"),
            ESearchCase::IgnoreCase);
    }

    bool IsBuildingMesh(const UStaticMeshComponent* Mesh)
    {
        if (!IsValid(Mesh))
        {
            return false;
        }

        const UStaticMesh* Asset = Mesh->GetStaticMesh();
        return Asset &&
            (Asset->GetPathName().Contains(TEXT("/Buildings/"),
                ESearchCase::IgnoreCase) ||
             Asset->GetName().Contains(TEXT("building"),
                ESearchCase::IgnoreCase));
    }

    bool HasFinalBuildingTag(const AActor* Actor)
    {
        return IsValid(Actor) &&
            (Actor->ActorHasTag(TEXT("FinalBuilding")) ||
             Actor->ActorHasTag(TEXT("WinTarget")));
    }

    bool IsFinalBuildingMesh(const UStaticMeshComponent* Mesh)
    {
        if (!IsValid(Mesh))
        {
            return false;
        }

        if (IsFinalBuildingName(Mesh->GetName()))
        {
            return true;
        }

        if (Mesh->ComponentHasTag(TEXT("FinalBuilding")) ||
            Mesh->ComponentHasTag(TEXT("WinTarget")))
        {
            return true;
        }

        const UStaticMesh* Asset = Mesh->GetStaticMesh();
        if (Asset && IsFinalBuildingName(Asset->GetName()))
        {
            return true;
        }

        return HasFinalBuildingTag(Mesh->GetOwner());
    }
}

UClockEaterComponent::UClockEaterComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UClockEaterComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    TArray<UStaticMeshComponent*> Meshes;
    Owner->GetComponents<UStaticMeshComponent>(Meshes);

    for (UStaticMeshComponent* Mesh : Meshes)
    {
        if (Mesh && (Mesh->ComponentHasTag(TEXT("ClockVisual")) ||
            Mesh->GetName().Contains(TEXT("ClockMesh"))))
        {
            ClockVisual = Mesh;
            break;
        }
    }
}

bool UClockEaterComponent::AddEdibles(int32 Count)
{
    AActor* Owner = GetOwner();
    UWorld* World = GetWorld();
    if (!IsValid(Owner) || !World || bHasFinishedLevel)
    {
        return false;
    }

    UClass* EdibleClass = LoadClass<AActor>(
        nullptr,
        TEXT("/Game/selfmade/edibles/BP_EdibleObject.BP_EdibleObject_C"));
    if (!EdibleClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Unable to load the edible blueprint class"));
        return false;
    }

    const int32 SafeCount = FMath::Clamp(Count, 1, 24);
    int32 SpawnedCount = 0;
    for (int32 Index = 0; Index < SafeCount; ++Index)
    {
        const float Angle = 360.0f * Index / SafeCount;
        const float Distance = 180.0f + 35.0f * (Index % 3);
        const FVector Offset(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            20.0f);
        FActorSpawnParameters SpawnParameters;
        SpawnParameters.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        AActor* Edible = World->SpawnActor<AActor>(
            EdibleClass,
            Owner->GetActorLocation() + Offset,
            FRotator::ZeroRotator,
            SpawnParameters);
        if (IsValid(Edible))
        {
            Edible->Tags.AddUnique(TEXT("Edible"));
            ++SpawnedCount;
        }
    }

    UE_LOG(LogTemp, Display, TEXT("Spawned %d refill edibles"), SpawnedCount);
    return SpawnedCount > 0;
}

void UClockEaterComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bHasFinishedLevel)
    {
        return;
    }

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

bool UClockEaterComponent::IsWinTarget(const AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return false;
    }

    if (IsFinalBuildingName(Actor->GetName()) ||
        HasFinalBuildingTag(Actor))
    {
        return true;
    }

    TArray<UStaticMeshComponent*> Meshes;
    Actor->GetComponents<UStaticMeshComponent>(Meshes);
    for (const UStaticMeshComponent* Mesh : Meshes)
    {
        if (IsFinalBuildingMesh(Mesh))
        {
            return true;
        }
    }

    // A placed static-mesh actor may have only a generated runtime name. In
    // that case, treat the largest city-building mesh as the final target.
    bool bHasBuildingMesh = false;
    float CandidateFootprint = 0.0f;
    for (const UStaticMeshComponent* Mesh : Meshes)
    {
        if (IsBuildingMesh(Mesh))
        {
            bHasBuildingMesh = true;
            CandidateFootprint = FMath::Max(
                CandidateFootprint,
                Mesh->Bounds.BoxExtent.X * Mesh->Bounds.BoxExtent.Y);
        }
    }

    if (!bHasBuildingMesh || CandidateFootprint <= 0.0f)
    {
        return false;
    }

    UWorld* World = Actor->GetWorld();
    if (!World)
    {
        return false;
    }

    float LargestFootprint = CandidateFootprint;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        const AActor* OtherActor = *It;
        if (!IsValid(OtherActor) || OtherActor == Actor)
        {
            continue;
        }

        TArray<UStaticMeshComponent*> OtherMeshes;
        OtherActor->GetComponents<UStaticMeshComponent>(OtherMeshes);
        for (const UStaticMeshComponent* Mesh : OtherMeshes)
        {
            if (IsBuildingMesh(Mesh))
            {
                LargestFootprint = FMath::Max(
                    LargestFootprint,
                    Mesh->Bounds.BoxExtent.X * Mesh->Bounds.BoxExtent.Y);
            }
        }
    }

    return CandidateFootprint >= LargestFootprint - 1.0f;
}

bool UClockEaterComponent::GetWinTargetFloorBounds(
    const AActor* Actor,
    FVector2D& OutCenter,
    FVector2D& OutExtent)
{
    if (!IsValid(Actor))
    {
        return false;
    }

    TArray<UStaticMeshComponent*> Meshes;
    Actor->GetComponents<UStaticMeshComponent>(Meshes);

    float LowestFloor = TNumericLimits<float>::Max();
    bool bFoundTargetMesh = false;
    for (const UStaticMeshComponent* Mesh : Meshes)
    {
        if (!IsFinalBuildingMesh(Mesh) && !IsBuildingMesh(Mesh))
        {
            continue;
        }

        const FBoxSphereBounds& Bounds = Mesh->Bounds;
        LowestFloor = FMath::Min(
            LowestFloor,
            Bounds.Origin.Z - Bounds.BoxExtent.Z);
        bFoundTargetMesh = true;
    }

    if (!bFoundTargetMesh)
    {
        return false;
    }

    FVector2D BoundsMin(TNumericLimits<float>::Max());
    FVector2D BoundsMax(TNumericLimits<float>::Lowest());
    const float FloorTolerance = 2.0f;
    for (const UStaticMeshComponent* Mesh : Meshes)
    {
        if (!IsFinalBuildingMesh(Mesh) && !IsBuildingMesh(Mesh))
        {
            continue;
        }

        const FBoxSphereBounds& Bounds = Mesh->Bounds;
        const float MeshFloor = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        if (MeshFloor > LowestFloor + FloorTolerance)
        {
            continue;
        }

        const FVector2D MeshMin(
            Bounds.Origin.X - Bounds.BoxExtent.X,
            Bounds.Origin.Y - Bounds.BoxExtent.Y);
        const FVector2D MeshMax(
            Bounds.Origin.X + Bounds.BoxExtent.X,
            Bounds.Origin.Y + Bounds.BoxExtent.Y);
        BoundsMin.X = FMath::Min(BoundsMin.X, MeshMin.X);
        BoundsMin.Y = FMath::Min(BoundsMin.Y, MeshMin.Y);
        BoundsMax.X = FMath::Max(BoundsMax.X, MeshMax.X);
        BoundsMax.Y = FMath::Max(BoundsMax.Y, MeshMax.Y);
    }

    if (BoundsMin.X > BoundsMax.X || BoundsMin.Y > BoundsMax.Y)
    {
        return false;
    }

    OutCenter = (BoundsMin + BoundsMax) * 0.5f;
    OutExtent = (BoundsMax - BoundsMin) * 0.5f;
    return true;
}

bool UClockEaterComponent::IsEdibleActor(const AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return false;
    }

    return Actor->ActorHasTag(TEXT("Edible")) ||
        Actor->GetName().Contains(TEXT("BP_Edible"), ESearchCase::IgnoreCase) ||
        IsChaosFood(Actor) ||
        IsWinTarget(Actor);
}

bool UClockEaterComponent::IsChaosFood(const AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return false;
    }

    if (Actor->GetName().Contains(TEXT("cactus"), ESearchCase::IgnoreCase))
    {
        return true;
    }

    TArray<UStaticMeshComponent*> Meshes;
    Actor->GetComponents<UStaticMeshComponent>(Meshes);
    for (const UStaticMeshComponent* Mesh : Meshes)
    {
        if (!IsValid(Mesh))
        {
            continue;
        }

        if (Mesh->GetName().Contains(TEXT("cactus"), ESearchCase::IgnoreCase))
        {
            return true;
        }

        const UStaticMesh* Asset = Mesh->GetStaticMesh();
        if (Asset && Asset->GetName().Contains(TEXT("SM_Bush_07"),
            ESearchCase::IgnoreCase))
        {
            return true;
        }
    }

    return false;
}

void UClockEaterComponent::FindEdible()
{
    AActor* Owner = GetOwner();
    UWorld* World = GetWorld();
    if (!Owner || !World)
    {
        return;
    }

    TArray<FOverlapResult> Results;
    FCollisionObjectQueryParams ObjectTypes;
    ObjectTypes.AddObjectTypesToQuery(ECC_WorldStatic);
    ObjectTypes.AddObjectTypesToQuery(ECC_WorldDynamic);
    ObjectTypes.AddObjectTypesToQuery(ECC_PhysicsBody);
    const float SearchRadius = FMath::Max(
        DetectionRadius, CurrentRadius * 2.f);
    World->OverlapMultiByObjectType(
        Results,
        Owner->GetActorLocation(),
        FQuat::Identity,
        ObjectTypes,
        FCollisionShape::MakeSphere(SearchRadius));

    for (const FOverlapResult& Result : Results)
    {
        AActor* Item = Result.GetActor();
        if (!IsValid(Item) || Item == Owner || !IsEdibleActor(Item))
        {
            continue;
        }

        if (CanSwallow(Owner, Item))
        {
            StartSwallow(Item);
            return;
        }
    }

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Item = *It;
        if (!IsValid(Item) || Item == Owner || !IsEdibleActor(Item))
        {
            continue;
        }

        const float Distance = FVector::Dist(
            Owner->GetActorLocation(), Item->GetActorLocation());
        if (Distance > SearchRadius)
        {
            continue;
        }

        if (CanSwallow(Owner, Item))
        {
            StartSwallow(Item);
            return;
        }
    }
}

bool UClockEaterComponent::CanSwallow(AActor* Owner, AActor* Item) const
{
    if (!IsValid(Owner) || !IsValid(Item) || !IsEdibleActor(Item))
    {
        return false;
    }

    if (IsWinTarget(Item))
    {
        FVector2D FloorCenter;
        FVector2D FloorExtent;
        if (!GetWinTargetFloorBounds(Item, FloorCenter, FloorExtent))
        {
            return false;
        }

        const FVector OwnerLocation = Owner->GetActorLocation();
        const float HorizontalDistance = FVector2D::Distance(
            FVector2D(OwnerLocation.X, OwnerLocation.Y),
            FloorCenter);
        const float BuildingFloorRadius = FMath::Max(
            FloorExtent.X,
            FloorExtent.Y);

        // Only the target mesh's floor footprint matters; building height is ignored.
        return FloorExtent.X > 0.f &&
            FloorExtent.Y > 0.f &&
            BuildingFloorRadius <= CurrentRadius &&
            HorizontalDistance + BuildingFloorRadius <= CurrentRadius;
    }

    FVector ItemOrigin;
    FVector ItemExtent;
    Item->GetActorBounds(false, ItemOrigin, ItemExtent);
    const float ItemRadius = ItemExtent.GetMax();
    if (ItemRadius <= 0.f || ItemRadius > CurrentRadius)
    {
        return false;
    }

    const float Distance = FVector::Dist(
        Owner->GetActorLocation(), ItemOrigin);
    return Distance <= CurrentRadius + ItemRadius;
}

void UClockEaterComponent::StartSwallow(AActor* Item)
{
    if (!IsValid(Item))
    {
        return;
    }

    EnsureEdibleVisual(Item);
    CurrentItem = Item;
    ItemStartLocation = Item->GetActorLocation();
    ItemStartScale = Item->GetActorScale3D();
    SwallowTime = 0.f;
    Item->SetActorEnableCollision(false);
}

void UClockEaterComponent::EnsureEdibleVisual(AActor* Actor)
{
    if (!IsValid(Actor) ||
        !Actor->GetName().Contains(TEXT("BP_Edible"), ESearchCase::IgnoreCase))
    {
        return;
    }

    TArray<UStaticMeshComponent*> Meshes;
    Actor->GetComponents<UStaticMeshComponent>(Meshes);
    for (UStaticMeshComponent* Mesh : Meshes)
    {
        if (!IsValid(Mesh) || Mesh->GetStaticMesh())
        {
            continue;
        }

        UStaticMesh* FallbackMesh = LoadObject<UStaticMesh>(
            nullptr,
            TEXT("/Game/selfmade/cools/powerup_random_1_LOD2.powerup_random_1_LOD2"));
        if (FallbackMesh)
        {
            Mesh->SetStaticMesh(FallbackMesh);
        }
        return;
    }
}

void UClockEaterComponent::UpdateSwallow(float DeltaTime)
{
    if (!CurrentItem.IsValid())
    {
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    SwallowTime += DeltaTime;
    const float Alpha = FMath::Clamp(
        SwallowTime / FMath::Max(SwallowDuration, 0.01f), 0.f, 1.f);
    const float SmoothAlpha = FMath::InterpEaseInOut(0.f, 1.f, Alpha, 2.f);
    const FVector TargetLocation =
        Owner->GetActorLocation() + FVector(0.f, 0.f, -35.f);

    CurrentItem->SetActorLocation(
        FMath::Lerp(ItemStartLocation, TargetLocation, SmoothAlpha));
    CurrentItem->SetActorScale3D(
        FMath::Lerp(ItemStartScale, ItemStartScale * 0.03f, SmoothAlpha));

    if (Alpha >= 1.f)
    {
        FinishSwallow();
    }
}

void UClockEaterComponent::FinishSwallow()
{
    AActor* Item = CurrentItem.Get();
    AActor* Owner = GetOwner();
    if (!IsValid(Item) || !IsValid(Owner))
    {
        CurrentItem.Reset();
        return;
    }

    const bool bChaosFood = IsChaosFood(Item);
    UPrimitiveComponent* ItemMesh = Cast<UPrimitiveComponent>(Item->GetRootComponent());
    if (!ItemMesh)
    {
        TArray<UPrimitiveComponent*> Primitives;
        Item->GetComponents<UPrimitiveComponent>(Primitives);
        ItemMesh = Primitives.Num() > 0 ? Primitives[0] : nullptr;
    }

    int32 PointsEarned = 0;
    float GrowthAmount = 0.f;
    UClockGrowth::CalculateEatReward(ItemMesh, PointsEarned, GrowthAmount);
    if (bChaosFood)
    {
        CurrentRadius = UClockGrowth::SetRadius(
            Owner, CurrentRadius, PreviousRadius * 0.75f);
    }
    else
    {
        PreviousRadius = CurrentRadius;
        CurrentRadius = UClockGrowth::FinishEating(
            Owner,
            CurrentRadius,
            GrowthAmount * GrowthMultiplier);
    }
    Score += PointsEarned;
    if (!bChaosFood)
    {
        MaxEdibleSize *= GrowthMultiplier;
    }
    DetectionRadius *= 1.05f;

    if (IsWinTarget(Item))
    {
        bHasFinishedLevel = true;
        if (UWorld* World = GetWorld())
        {
            if (UClockStatsSubsystem* Stats =
                World->GetSubsystem<UClockStatsSubsystem>())
            {
                Stats->RecordWin();
            }
        }
        UE_LOG(LogTemp, Display, TEXT("Win target swallowed; level complete"));
    }

    Item->Destroy();
    CurrentItem.Reset();

    if (bChaosFood)
    {
        if (AClockPawn* Clock = Cast<AClockPawn>(Owner))
        {
            Clock->TriggerMovementChaos(10.f);
        }
        else if (AProject3v1Character* Character =
            Cast<AProject3v1Character>(Owner))
        {
            Character->TriggerMovementChaos(10.f);
        }
    }
}
