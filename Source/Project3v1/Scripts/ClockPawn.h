
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ClockPawn.generated.h"

class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UFloatingPawnMovement;

UCLASS()
class PROJECT3V1_API AClockPawn : public APawn
{
    GENERATED_BODY()

public:
    AClockPawn();

    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(
        UInputComponent* PlayerInputComponent) override;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* ClockMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USpringArmComponent* SpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UCameraComponent* Camera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UFloatingPawnMovement* Movement;

    UPROPERTY(EditAnywhere, Category="Movement")
    float MoveSpeed = 600.f;

    void MoveForward(float Value);
    void MoveRight(float Value);
};
