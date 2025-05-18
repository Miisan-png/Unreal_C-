#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "ItemNameWidget.h"
#include "MyFPSCharacter.generated.h"

class AItemManager;

UCLASS()
class FIRSTPERSONTEST_API AMyFPSCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AMyFPSCharacter();

    virtual void Tick(float DeltaTime) override;

    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UFUNCTION(BlueprintCallable, Category = "Camera")
    UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }

    UFUNCTION(BlueprintCallable, Category = "Raycast")
    float GetRaycastDistance() const { return RaycastDistance; }

    void Interact();

protected:
    virtual void BeginPlay() override;

    void MoveForward(float Value);
    void MoveRight(float Value);

    void StartSprint();
    void StopSprint();
    float DefaultMaxWalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics")
    bool bEnablePhysicsInteraction = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics")
    float PushForceFactor = 500000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics")
    float PushForcePointZOffset = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Sway")
    float PositionalSwayAmount = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Sway")
    float RotationalSwayAmount = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Sway")
    float SwaySpeed = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Sway")
    float ReturnSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Sway")
    float LandingBobAmount = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Sway")
    float BreathingAmount = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Sway")
    float BreathingSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UItemNameWidget> ItemNameWidgetClass;

    UPROPERTY()
    UItemNameWidget* ItemNameWidget;
    
    FRotator InitialCameraRotation;
    FVector InitialCameraLocation;
    FRotator CurrentRotationalSway;
    FVector CurrentPositionalSway;
    float BreathingPhase;
    float LastZVelocity;
    bool bWasInAir;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float SprintSpeedMultiplier = 1.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsSprinting = false;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FirstPersonCamera;
    
    void HandleCameraSway(float DeltaTime);
    
    void PerformCameraRaycast();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raycast", meta = (AllowPrivateAccess = "true"))
    float RaycastDistance = 1000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raycast", meta = (AllowPrivateAccess = "true"))
    float RaycastDebugDuration = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raycast", meta = (AllowPrivateAccess = "true"))
    bool bShowDebugRaycast = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raycast|Colors", meta = (AllowPrivateAccess = "true"))
    FColor RaycastHitColor = FColor::Red;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raycast|Colors", meta = (AllowPrivateAccess = "true"))
    FColor RaycastMissColor = FColor::Blue;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raycast|Colors", meta = (AllowPrivateAccess = "true"))
    FColor HitPointColor = FColor::Green;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Interaction", meta = (AllowPrivateAccess = "true"))
    USceneComponent* ItemHoldPoint;
    
    UPROPERTY()
    AItemManager* ItemManagerRef;
};