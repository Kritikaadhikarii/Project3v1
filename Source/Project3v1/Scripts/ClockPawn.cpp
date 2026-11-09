
#include "ClockPawn.h"

#include "ClockEaterComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Camera/CameraComponent.h"

AClockPawn::AClockPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    ClockMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        TEXT("ClockMesh"));
    RootComponent = ClockMesh;

    ClockMesh->SetCollisionProfileName(TEXT("Pawn"));

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(
        TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 900.f;
    SpringArm->SetRelativeRotation(
        FRotator(-60.f, 0.f, 0.f));
    SpringArm->bDoCollisionTest = false;

    Camera = CreateDefaultSubobject<UCameraComponent>(
        TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);

    Movement = CreateDefaultSubobject<UFloatingPawnMovement>(
        TEXT("Movement"));
    Movement->MaxSpeed = 600.f;

    Eater = CreateDefaultSubobject<UClockEaterComponent>(TEXT("Eater"));

    for (int32 Index = 0; Index < 3; ++Index)
    {
        UStaticMeshComponent* Fragment = CreateDefaultSubobject<UStaticMeshComponent>(
            *FString::Printf(TEXT("ClockFragment%d"), Index));
        Fragment->SetupAttachment(RootComponent);
        Fragment->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Fragment->SetVisibility(false);
        ClockFragments.Add(Fragment);
    }

    AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void AClockPawn::BeginPlay()
{
    Super::BeginPlay();
}

void AClockPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (ChaosTimeRemaining > 0.f)
    {
        ChaosTimeRemaining = FMath::Max(0.f, ChaosTimeRemaining - DeltaTime);
        ChaosTime += DeltaTime;
        AddMovementInput(
            FVector(
                FMath::Sin(ChaosTime * 5.f),
                FMath::Cos(ChaosTime * 7.f),
                0.f),
            1.0f);

        if (ChaosTimeRemaining <= 0.f)
        {
            SetClockFragmentsVisible(false);
            ClockMesh->SetVisibility(true);
        }
    }
}

void AClockPawn::SetupPlayerInputComponent(
    UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis(
        TEXT("MoveForward"), this, &AClockPawn::MoveForward);

    PlayerInputComponent->BindAxis(
        TEXT("MoveRight"), this, &AClockPawn::MoveRight);
}

void AClockPawn::MoveForward(float Value)
{
    if (!FMath::IsNearlyZero(Value) && ChaosTimeRemaining <= 0.f)
    {
        AddMovementInput(FVector::ForwardVector, Value);
    }
    else if (!FMath::IsNearlyZero(Value))
    {
        const float Angle = ChaosTime * 11.f;
        const FVector Input = FVector::ForwardVector * Value;
        const FVector Sideways = FVector::RightVector *
            (FMath::Sin(ChaosTime * 17.f) * 0.8f);
        AddMovementInput(Input.RotateAngleAxis(Angle, FVector::UpVector) + Sideways);
    }
}

void AClockPawn::MoveRight(float Value)
{
    if (!FMath::IsNearlyZero(Value) && ChaosTimeRemaining <= 0.f)
    {
        AddMovementInput(FVector::RightVector, Value);
    }
    else if (!FMath::IsNearlyZero(Value))
    {
        const float Angle = ChaosTime * 13.f;
        const FVector Input = FVector::RightVector * Value;
        const FVector Forward = FVector::ForwardVector *
            (FMath::Cos(ChaosTime * 19.f) * 0.8f);
        AddMovementInput(Input.RotateAngleAxis(Angle, FVector::UpVector) + Forward);
    }
}

void AClockPawn::TriggerMovementChaos(float Duration)
{
    ChaosTimeRemaining = FMath::Max(ChaosTimeRemaining, Duration);
    ChaosTime = 0.f;
    ClockMesh->SetVisibility(false);

    for (int32 Index = 0; Index < ClockFragments.Num(); ++Index)
    {
        UStaticMeshComponent* Fragment = ClockFragments[Index];
        if (!Fragment)
        {
            continue;
        }

        Fragment->SetStaticMesh(ClockMesh->GetStaticMesh());
        const float Angle = 120.f * Index;
        Fragment->SetRelativeLocation(
            FVector(FMath::Cos(FMath::DegreesToRadians(Angle)) * 35.f,
                FMath::Sin(FMath::DegreesToRadians(Angle)) * 35.f, 0.f));
        Fragment->SetRelativeScale3D(ClockMesh->GetRelativeScale3D() * 0.65f);
        Fragment->SetVisibility(true);
    }
}

void AClockPawn::SetClockFragmentsVisible(bool bVisible)
{
    for (UStaticMeshComponent* Fragment : ClockFragments)
    {
        if (Fragment)
        {
            Fragment->SetVisibility(bVisible);
        }
    }
}
