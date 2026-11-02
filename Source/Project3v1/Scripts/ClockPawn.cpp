
#include "ClockPawn.h"

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

    AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void AClockPawn::BeginPlay()
{
    Super::BeginPlay();
}

void AClockPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
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
    if (!FMath::IsNearlyZero(Value))
    {
        AddMovementInput(FVector::ForwardVector, Value);
    }
}

void AClockPawn::MoveRight(float Value)
{
    if (!FMath::IsNearlyZero(Value))
    {
        AddMovementInput(FVector::RightVector, Value);
    }
}
