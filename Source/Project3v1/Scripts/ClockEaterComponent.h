
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ClockEaterComponent.generated.h"

class UStaticMeshComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT3V1_API UClockEaterComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UClockEaterComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(
        float DeltaTime,
        ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction
    ) override;

    UFUNCTION(BlueprintCallable, Category = "Clock Eater")
    bool AddEdibles(int32 Count = 8);

    UFUNCTION(BlueprintPure, Category = "Clock Eater")
    bool HasFinishedLevel() const { return bHasFinishedLevel; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock Eater")
    float DetectionRadius = 160.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock Eater")
    float MaxEdibleSize = 120.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock Eater")
    float SwallowDuration = 0.65f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock Eater")
    float GrowthMultiplier = 1.12f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock Eater")
    float CurrentRadius = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock Eater")
    float PreviousRadius = 100.f;

    UPROPERTY(BlueprintReadOnly, Category = "Clock Eater")
    int32 Score = 0;

private:
    UPROPERTY()
    TObjectPtr<UStaticMeshComponent> ClockVisual;

    TWeakObjectPtr<AActor> CurrentItem;

    FVector ItemStartLocation;
    FVector ItemStartScale;

    float SwallowTime = 0.f;
    float SearchTimer = 0.f;
    bool bHasFinishedLevel = false;

    void FindEdible();
    void StartSwallow(AActor* Item);
    void UpdateSwallow(float DeltaTime);
    void FinishSwallow();
    static void EnsureEdibleVisual(AActor* Actor);
    static bool IsEdibleActor(const AActor* Actor);
    static bool IsChaosFood(const AActor* Actor);
    static bool IsWinTarget(const AActor* Actor);
    bool CanSwallow(AActor* Owner, AActor* Item) const;
};
