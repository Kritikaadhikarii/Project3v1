
#include "ClockBlackoutSubsystem.h"
#include "ClockBlackoutWidget.h"

#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"

void UClockBlackoutSubsystem::Initialize(
    FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("CLOCK BLACKOUT SUBSYSTEM INITIALIZED!")
    );
}
// ==================================================
// FUNCTION 1: Identify lantern by Static Mesh asset
// ==================================================

bool UClockBlackoutSubsystem::IsLantern(
    AActor* Actor) const
{
    if (!IsValid(Actor))
    {
        return false;
    }

    const FString ActorName = Actor->GetName();
    if (ActorName.Contains(TEXT("latern"), ESearchCase::IgnoreCase) ||
        ActorName.Contains(TEXT("lantern"), ESearchCase::IgnoreCase))
    {
        return true;
    }

    TArray<UStaticMeshComponent*> Meshes;
    Actor->GetComponents<UStaticMeshComponent>(Meshes);

    for (UStaticMeshComponent* Mesh : Meshes)
    {
        if (!IsValid(Mesh))
        {
            continue;
        }

        const FString MeshName = Mesh->GetName();
        if (MeshName.Contains(TEXT("latern"), ESearchCase::IgnoreCase) ||
            MeshName.Contains(TEXT("lantern"), ESearchCase::IgnoreCase))
        {
            return true;
        }

        UStaticMesh* MeshAsset = Mesh->GetStaticMesh();

        if (!IsValid(MeshAsset))
        {
            continue;
        }

        const FString AssetName = MeshAsset->GetName();

        if (AssetName.Contains(TEXT("latern"), ESearchCase::IgnoreCase) ||
            AssetName.Contains(TEXT("lantern"), ESearchCase::IgnoreCase))
        {
            return true;
        }
    }

    return false;
}


// ==================================================
// FUNCTION 2: Register all lanterns at level startup
// ==================================================

void UClockBlackoutSubsystem::OnWorldBeginPlay(
    UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (!InWorld.IsGameWorld())
    {
        return;
    }

    int32 LanternCount = 0;

    for (TActorIterator<AActor> It(&InWorld); It; ++It)
    {
        AActor* Actor = *It;

        if (!IsValid(Actor))
        {
            continue;
        }

        if (!IsLantern(Actor))
        {
            continue;
        }

        // Register destruction event once per lantern.
        Actor->OnDestroyed.AddUniqueDynamic(
            this,
            &UClockBlackoutSubsystem::OnLanternDestroyed
        );

        LanternCount++;

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Registered blackout lantern: %s"),
            *Actor->GetName()
        );
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("BLACKOUT SYSTEM READY: %d lantern(s)"),
        LanternCount
    );
}


// ==================================================
// FUNCTION 3: Called when a lantern actor is destroyed
// ==================================================

void UClockBlackoutSubsystem::OnLanternDestroyed(
    AActor* DestroyedActor)
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("LANTERN DESTROYED: %s"),
        *GetNameSafe(DestroyedActor)
    );

    TriggerBlackout();
}


// ==================================================
// FUNCTION 4: Start six-second blackout
// ==================================================

void UClockBlackoutSubsystem::TriggerBlackout()
{
    StartBlackout();
}

void UClockBlackoutSubsystem::StartBlackout()
{
    UWorld* World = GetWorld();

    if (!World || !World->IsGameWorld())
    {
        return;
    }

    APlayerController* PC =
        World->GetFirstPlayerController();

    if (!IsValid(PC) ||
        !IsValid(PC->PlayerCameraManager))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Blackout failed: Camera manager unavailable")
        );
        return;
    }

    if (!BlackoutWidget)
    {
        BlackoutWidget = CreateWidget<UClockBlackoutWidget>(
            PC,
            UClockBlackoutWidget::StaticClass()
        );

        if (BlackoutWidget)
        {
            BlackoutWidget->AddToViewport(100);
            BlackoutWidget->SetPositionInViewport(FVector2D(0.0f, 0.0f));
            BlackoutWidget->SetDesiredSizeInViewport(FVector2D(500.0f, 250.0f));
        }
    }

    if (BlackoutWidget)
    {
        BlackoutWidget->SetVisibility(ESlateVisibility::Visible);
        BlackoutWidget->StartCountdown(BlackoutDuration);
    }

    // Screen becomes fully black immediately.
    PC->PlayerCameraManager->SetManualCameraFade(
        1.0f,
        FLinearColor::Black,
        false
    );
    PC->PlayerCameraManager->StartCameraFade(
        1.0f,
        1.0f,
        BlackoutDuration,
        FLinearColor::Black,
        false,
        true
    );

    // Restart the timer for each lantern.
    World->GetTimerManager().ClearTimer(BlackoutTimer);

    World->GetTimerManager().SetTimer(
        BlackoutTimer,
        this,
        &UClockBlackoutSubsystem::EndBlackout,
        BlackoutDuration,
        false
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("BLACKOUT STARTED FOR 6 SECONDS")
    );
}


// ==================================================
// FUNCTION 5: Restore vision after blackout
// ==================================================

void UClockBlackoutSubsystem::EndBlackout()
{
    UWorld* World = GetWorld();

    if (!World)
    {
        return;
    }

    APlayerController* PC =
        World->GetFirstPlayerController();

    if (IsValid(PC) &&
        IsValid(PC->PlayerCameraManager))
    {
        PC->PlayerCameraManager->SetManualCameraFade(
            0.0f,
            FLinearColor::Black,
            false
        );
        PC->PlayerCameraManager->StopCameraFade();
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("BLACKOUT ENDED - VISION RESTORED")
    );
}
