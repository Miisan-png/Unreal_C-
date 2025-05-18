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

    UPROPERTY(EditAnywhere, Category = "Movement")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float YawOffset;

    UPROPERTY(EditAnywhere, Category = "Movement")
    bool bCanMove;

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
    USceneComponent* Root;

    bool bShouldBreak;
    bool bBroken;

    FTimerHandle DebugBreakTimerHandle;
};
