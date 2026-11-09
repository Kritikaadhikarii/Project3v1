
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "ClockBlackoutSubsystem.generated.h"

class AActor;
class UClockBlackoutWidget;

UCLASS()
class UClockBlackoutSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:

    // Called when Unreal initializes the subsystem.
    virtual void Initialize(
        FSubsystemCollectionBase& Collection
    ) override;

    // Called when the game world begins playing.
    virtual void OnWorldBeginPlay(
        UWorld& InWorld
    ) override;

    // Starts a full-screen blackout and restarts the six-second duration.
    void TriggerBlackout();

private:

    // Called when a registered lantern is destroyed.
    UFUNCTION()
    void OnLanternDestroyed(AActor* DestroyedActor);

    // Checks whether an actor uses the lantern mesh.
    bool IsLantern(AActor* Actor) const;

    // Controls the blackout.
    void StartBlackout();
    void EndBlackout();

    // Blackout timer.
    FTimerHandle BlackoutTimer;

    UPROPERTY()
    TObjectPtr<UClockBlackoutWidget> BlackoutWidget;

    // Duration in seconds.
    static constexpr float BlackoutDuration = 6.0f;
};
