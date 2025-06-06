#include "WheepingAngle.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/KismetMathLibrary.h"

AWheepingAngle::AWheepingAngle()
{
    PrimaryActorTick.bCanEverTick = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    AngelMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AngelMeshComponent"));
    AngelMeshComponent->SetupAttachment(RootComponent);

    YawOffset = 90.0f;
    TeleportDistance = 300.0f;
    TeleportCooldown = 2.0f;
    bCanTeleport = true;

    bShouldBreak = false;
    bBroken = false;
    DestroyDelay = 5.0f;

    MinShards = 3;
    MaxShards = 6;
    MinScale = 0.5f;
    MaxScale = 1.5f;

    bEnableDebugDestroy = false;
    DebugBreakDelay = 3.0f;
    
    LastTeleportTime = 0.0f;
}

AWheepingAngle::~AWheepingAngle() = default;

void AWheepingAngle::BeginPlay()
{
    Super::BeginPlay();

    if (bEnableDebugDestroy)
    {
        GetWorld()->GetTimerManager().SetTimer(
            DebugBreakTimerHandle,
            [this]() { bShouldBreak = true; },
            DebugBreakDelay,
            false
        );
    }
}

void AWheepingAngle::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bShouldBreak && !bBroken)
    {
        bBroken = true;
        BreakAngel();
    }

    if (!bBroken)
    {
        APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
        if (PlayerController)
        {
            FVector PlayerLocation;
            FRotator PlayerRotation;
            PlayerController->GetPlayerViewPoint(PlayerLocation, PlayerRotation);

            FVector PlayerForward = PlayerRotation.Vector();
            FVector DirectionToActor = GetActorLocation() - PlayerLocation;
            DirectionToActor.Normalize();

            float LookDot = FVector::DotProduct(PlayerForward, DirectionToActor);
            float LookThreshold = 0.7f;

            if (LookDot < LookThreshold && bCanTeleport)
            {
                float CurrentTime = GetWorld()->GetTimeSeconds();
                if (CurrentTime - LastTeleportTime >= TeleportCooldown)
                {
                    TeleportBehindPlayer(PlayerLocation, PlayerRotation);
                    LastTeleportTime = CurrentTime;
                }
            }
            
            if (LookDot >= LookThreshold)
            {
                FVector ToPlayer = PlayerLocation - GetActorLocation();
                ToPlayer.Z = 0.0f;
                ToPlayer.Normalize();
                FRotator LookAtRotation = FRotationMatrix::MakeFromX(ToPlayer).Rotator();
                LookAtRotation.Yaw += YawOffset;
                LookAtRotation.Yaw = FMath::Fmod(LookAtRotation.Yaw + 360.f, 360.f);
                SetActorRotation(FRotator(0.f, LookAtRotation.Yaw, 0.f));
            }
        }
    }
}

void AWheepingAngle::TeleportBehindPlayer(const FVector& PlayerLocation, const FRotator& PlayerRotation)
{
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn) return;
    
    FVector PlayerForward = PlayerPawn->GetActorForwardVector();
    PlayerForward.Z = 0.0f;
    PlayerForward.Normalize();
    
    FVector TeleportLocation = PlayerLocation + (PlayerForward * TeleportDistance * DirectionMultiplier);
    TeleportLocation.Z = GetActorLocation().Z;
    
    SetActorLocation(TeleportLocation);
    
    FVector ToPlayer = PlayerLocation - TeleportLocation;
    ToPlayer.Z = 0.0f;
    ToPlayer.Normalize();
    FRotator LookAtRotation = FRotationMatrix::MakeFromX(ToPlayer).Rotator();
    LookAtRotation.Yaw += YawOffset;
    LookAtRotation.Yaw = FMath::Fmod(LookAtRotation.Yaw + 360.f, 360.f);
    SetActorRotation(FRotator(0.f, LookAtRotation.Yaw, 0.f));
}

void AWheepingAngle::BreakAngel()
{
    if (UWorld* World = GetWorld())
    {
        FVector Origin = GetActorLocation();
        int32 NumShards = FMath::RandRange(MinShards, MaxShards);

        for (int i = 0; i < NumShards; ++i)
        {
            if (ShardMeshes.Num() == 0) continue;

            int32 MeshIndex = FMath::RandRange(0, ShardMeshes.Num() - 1);
            UStaticMesh* Mesh = ShardMeshes[MeshIndex];
            if (!Mesh) continue;

            FVector Offset = FVector(
                FMath::FRandRange(-50.f, 50.f),
                FMath::FRandRange(-50.f, 50.f),
                FMath::FRandRange(-10.f, 10.f)
            );
            FVector SpawnLocation = Origin + Offset;

            FActorSpawnParameters SpawnParams;
            AStaticMeshActor* Shard = World->SpawnActor<AStaticMeshActor>(SpawnLocation, FRotator::ZeroRotator, SpawnParams);

            if (Shard)
            {
                UStaticMeshComponent* MeshComp = Shard->GetStaticMeshComponent();
                MeshComp->SetMobility(EComponentMobility::Movable);
                MeshComp->SetStaticMesh(Mesh);
                MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                MeshComp->SetSimulatePhysics(true);

                float Scale = FMath::FRandRange(MinScale, MaxScale);
                Shard->SetActorScale3D(FVector(Scale));

                FVector Impulse = FVector(
                    FMath::FRandRange(-300.f, 300.f),
                    FMath::FRandRange(-300.f, 300.f),
                    FMath::FRandRange(50.f, 150.f)
                );
                MeshComp->AddImpulse(Impulse, NAME_None, true);

                Shard->SetLifeSpan(DestroyDelay);

                DrawDebugSphere(World, SpawnLocation, 15.f, 12, FColor::Red, false, 2.f);
            }
        }

        AngelMeshComponent->SetVisibility(false);
        SetActorEnableCollision(false);
        GetWorldTimerManager().SetTimerForNextTick(this, &AWheepingAngle::DestroyAngel);
    }
}

void AWheepingAngle::DestroyAngel()
{
    SetLifeSpan(DestroyDelay);
}

UStaticMeshComponent* AWheepingAngle::GetAngelMeshComponent() const
{
    return AngelMeshComponent;
}