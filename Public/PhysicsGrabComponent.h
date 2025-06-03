#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Components/PrimitiveComponent.h"
#include "Sound/SoundBase.h"
#include "PhysicsGrabComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FIRSTPERSONTEST_API UPhysicsGrabComponent : public UActorComponent
{
    GENERATED_BODY()

public:    
    UPhysicsGrabComponent();

protected:
    virtual void BeginPlay() override;

public:    
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Grab Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Grab")
    float GrabRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Grab")
    float GrabDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Grab")
    float GrabForce = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Grab")
    float MaxGrabMass = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Grab")
    float ThrowForce = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Grab")
    float RotationSpeed = 90.0f;

    // Visual Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Grab")
    bool bShowGrabPoint = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Grab")
    bool bShowDebugLines = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Grab")
    UMaterialInterface* GrabHighlightMaterial;

    // Audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Grab")
    USoundBase* GrabSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Grab")
    USoundBase* ReleaseSound;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Physics Grab")
    void StartGrab();

    UFUNCTION(BlueprintCallable, Category = "Physics Grab")
    void StopGrab();

    UFUNCTION(BlueprintCallable, Category = "Physics Grab")
    void ThrowObject();

    UFUNCTION(BlueprintCallable, Category = "Physics Grab")
    void RotateGrabbedObject(float PitchInput, float YawInput);

    UFUNCTION(BlueprintCallable, Category = "Physics Grab")
    bool IsGrabbing() const { return bIsGrabbing; }

    UFUNCTION(BlueprintCallable, Category = "Physics Grab")
    AActor* GetGrabbedActor() const { return GrabbedActor; }

protected:
    // Internal state
    UPROPERTY()
    bool bIsGrabbing = false;

    UPROPERTY()
    AActor* GrabbedActor = nullptr;

    UPROPERTY()
    UPrimitiveComponent* GrabbedComponent = nullptr;

    UPROPERTY()
    AActor* HighlightedActor = nullptr;

    UPROPERTY()
    UMaterialInterface* OriginalMaterial = nullptr;

    UPROPERTY()
    FVector GrabOffset;

    UPROPERTY()
    FVector LastPlayerLocation;

    UPROPERTY()
    float CurrentRotationPitch = 0.0f;

    UPROPERTY()
    float CurrentRotationYaw = 0.0f;

    // Internal functions
    void PerformGrabTrace();
    void UpdateGrabbedObject(float DeltaTime);
    void SetObjectHighlight(AActor* Actor, bool bHighlight);
    bool CanGrabObject(AActor* Actor, UPrimitiveComponent* Component) const;
    FVector GetGrabTargetLocation() const;
    void ApplyGrabForce(float DeltaTime);
    UCameraComponent* GetPlayerCamera() const;
    APlayerController* GetPlayerController() const;
};