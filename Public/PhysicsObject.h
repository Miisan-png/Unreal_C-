#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Sound/SoundBase.h"
#include "PhysicsObject.generated.h"

UCLASS()
class FIRSTPERSONTEST_API APhysicsObject : public AActor
{
    GENERATED_BODY()
    
public:    
    APhysicsObject();

protected:
    virtual void BeginPlay() override;

public:
    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ObjectMesh;

    // Physics Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float ObjectMass = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bCanBeDragged = true;

    // Visual Feedback
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    UMaterialInterface* HighlightMaterial;

    UPROPERTY()
    UMaterialInterface* OriginalMaterial;

    // Audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* CollisionSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MinCollisionVelocity = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float CollisionSoundCooldown = 0.5f;

    UPROPERTY()
    float LastCollisionSoundTime = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void Highlight(bool bHighlight);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    bool CanBeDragged() const { return bCanBeDragged; }

protected:
    // Collision sounds
    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};