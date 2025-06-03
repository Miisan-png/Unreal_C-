#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WheepingAngle.generated.h"

UCLASS()
class FIRSTPERSONTEST_API AWheepingAngle : public AActor
{
    GENERATED_BODY()

public:
    AWheepingAngle();
    virtual ~AWheepingAngle();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void BreakAngel();

    UFUNCTION()
    void DestroyAngel();

    UStaticMeshComponent* GetAngelMeshComponent() const;

    UPROPERTY(EditAnywhere, Category = "Components")
    UStaticMeshComponent* AngelMeshComponent;

    UPROPERTY(EditAnywhere, Category = "Teleport")
    float TeleportDistance = 300.0f;

    UPROPERTY(EditAnywhere, Category = "Teleport")
    float TeleportCooldown = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Teleport")
    float YawOffset;

    UPROPERTY(EditAnywhere, Category = "Teleport")
    bool bCanTeleport;

    UPROPERTY(EditAnywhere, Category = "Teleport")
    float DirectionMultiplier = -1.0f;

    UPROPERTY(EditAnywhere, Category = "Destruction")
    float DestroyDelay;

    UPROPERTY(EditAnywhere, Category = "Destruction")
    TArray<UStaticMesh*> ShardMeshes;

    UPROPERTY(EditAnywhere, Category = "Destruction")
    int32 MinShards;

    UPROPERTY(EditAnywhere, Category = "Destruction")
    int32 MaxShards;

    UPROPERTY(EditAnywhere, Category = "Destruction")
    float MinScale;

    UPROPERTY(EditAnywhere, Category = "Destruction")
    float MaxScale;

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bEnableDebugDestroy;

    UPROPERTY(EditAnywhere, Category = "Debug")
    float DebugBreakDelay;

private:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    USceneComponent* Root;

    bool bShouldBreak;
    bool bBroken;
    float LastTeleportTime;

    FTimerHandle DebugBreakTimerHandle;

    void TeleportBehindPlayer(const FVector& PlayerLocation, const FRotator& PlayerRotation);
};