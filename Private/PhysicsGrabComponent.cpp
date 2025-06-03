#include "PhysicsGrabComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "PickableItem.h"
#include "PhysicsObject.h"

UPhysicsGrabComponent::UPhysicsGrabComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UPhysicsGrabComponent::BeginPlay()
{
    Super::BeginPlay();
    LastPlayerLocation = GetOwner()->GetActorLocation();
}

void UPhysicsGrabComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsGrabbing && GrabbedActor)
    {
        UpdateGrabbedObject(DeltaTime);
    }
    else
    {
        PerformGrabTrace();
    }

    LastPlayerLocation = GetOwner()->GetActorLocation();
}

void UPhysicsGrabComponent::PerformGrabTrace()
{
    UCameraComponent* Camera = GetPlayerCamera();
    if (!Camera)
        return;

    FVector CameraLocation = Camera->GetComponentLocation();
    FVector CameraForward = Camera->GetForwardVector();
    FVector TraceEnd = CameraLocation + (CameraForward * GrabRange);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.bTraceComplex = false;

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        CameraLocation,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    if (bShowDebugLines)
    {
        DrawDebugLine(
            GetWorld(),
            CameraLocation,
            bHit ? HitResult.ImpactPoint : TraceEnd,
            bHit ? FColor::Green : FColor::Red,
            false,
            0.1f,
            0,
            2.0f
        );
    }

    AActor* NewHighlightedActor = nullptr;
    
    if (bHit && HitResult.GetActor())
    {
        AActor* HitActor = HitResult.GetActor();
        UPrimitiveComponent* HitComponent = HitResult.GetComponent();
        
        if (CanGrabObject(HitActor, HitComponent))
        {
            NewHighlightedActor = HitActor;
            
            if (bShowGrabPoint)
            {
                DrawDebugSphere(
                    GetWorld(),
                    HitResult.ImpactPoint,
                    10.0f,
                    12,
                    FColor::Yellow,
                    false,
                    0.1f
                );
            }
        }
    }

    if (NewHighlightedActor != HighlightedActor)
    {
        if (HighlightedActor)
        {
            SetObjectHighlight(HighlightedActor, false);
        }
        
        HighlightedActor = NewHighlightedActor;
        
        if (HighlightedActor)
        {
            SetObjectHighlight(HighlightedActor, true);
        }
    }
}

void UPhysicsGrabComponent::UpdateGrabbedObject(float DeltaTime)
{
    if (!GrabbedActor || !GrabbedComponent)
    {
        StopGrab();
        return;
    }

    ApplyGrabForce(DeltaTime);

    if (bShowDebugLines)
    {
        FVector TargetLocation = GetGrabTargetLocation();
        FVector CurrentLocation = GrabbedComponent->GetComponentLocation();
        
        DrawDebugLine(
            GetWorld(),
            CurrentLocation,
            TargetLocation,
            FColor::Blue,
            false,
            0.1f,
            0,
            3.0f
        );
        
        DrawDebugSphere(
            GetWorld(),
            TargetLocation,
            15.0f,
            12,
            FColor::Blue,
            false,
            0.1f
        );
    }
}

void UPhysicsGrabComponent::StartGrab()
{
    if (bIsGrabbing || !HighlightedActor)
        return;

    UCameraComponent* Camera = GetPlayerCamera();
    if (!Camera)
        return;

    FVector CameraLocation = Camera->GetComponentLocation();
    FVector CameraForward = Camera->GetForwardVector();
    FVector TraceEnd = CameraLocation + (CameraForward * GrabRange);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        CameraLocation,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    if (bHit && HitResult.GetActor() == HighlightedActor)
    {
        GrabbedActor = HighlightedActor;
        GrabbedComponent = HitResult.GetComponent();
        
        FVector ObjectCenter = GrabbedComponent->GetComponentLocation();
        GrabOffset = HitResult.ImpactPoint - ObjectCenter;
        
        bIsGrabbing = true;
        CurrentRotationPitch = 0.0f;
        CurrentRotationYaw = 0.0f;

        if (GrabSound)
        {
            UGameplayStatics::PlaySound2D(GetWorld(), GrabSound);
        }

        SetObjectHighlight(HighlightedActor, false);
        HighlightedActor = nullptr;
    }
}

void UPhysicsGrabComponent::StopGrab()
{
    if (!bIsGrabbing)
        return;

    bIsGrabbing = false;
    
    if (GrabbedActor)
    {
        if (ReleaseSound)
        {
            UGameplayStatics::PlaySound2D(GetWorld(), ReleaseSound);
        }
    }

    GrabbedActor = nullptr;
    GrabbedComponent = nullptr;
    GrabOffset = FVector::ZeroVector;
}

void UPhysicsGrabComponent::ThrowObject()
{
    if (!bIsGrabbing || !GrabbedComponent)
        return;

    UCameraComponent* Camera = GetPlayerCamera();
    if (!Camera)
        return;

    FVector ThrowDirection = Camera->GetForwardVector();
    FVector ThrowVelocity = ThrowDirection * ThrowForce;

    if (ACharacter* PlayerCharacter = Cast<ACharacter>(GetOwner()))
    {
        ThrowVelocity += PlayerCharacter->GetVelocity();
    }

    GrabbedComponent->AddImpulse(ThrowVelocity, NAME_None, true);

    StopGrab();
}

void UPhysicsGrabComponent::RotateGrabbedObject(float PitchInput, float YawInput)
{
    if (!bIsGrabbing || !GrabbedComponent)
        return;

    CurrentRotationPitch += PitchInput * RotationSpeed * GetWorld()->GetDeltaSeconds();
    CurrentRotationYaw += YawInput * RotationSpeed * GetWorld()->GetDeltaSeconds();

    FVector AngularVelocity = FVector(CurrentRotationPitch, CurrentRotationYaw, 0.0f);
    GrabbedComponent->SetPhysicsAngularVelocityInDegrees(AngularVelocity);
}

void UPhysicsGrabComponent::SetObjectHighlight(AActor* Actor, bool bHighlight)
{
    if (!Actor)
        return;

    if (APhysicsObject* PhysicsObj = Cast<APhysicsObject>(Actor))
    {
        PhysicsObj->Highlight(bHighlight);
        return;
    }

    if (APickableItem* PickableItem = Cast<APickableItem>(Actor))
    {
        PickableItem->Highlight(bHighlight);
        return;
    }

    UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        if (bHighlight)
        {
            if (!OriginalMaterial)
            {
                OriginalMaterial = MeshComp->GetMaterial(0);
            }
            if (GrabHighlightMaterial)
            {
                MeshComp->SetMaterial(0, GrabHighlightMaterial);
            }
        }
        else
        {
            if (OriginalMaterial)
            {
                MeshComp->SetMaterial(0, OriginalMaterial);
                OriginalMaterial = nullptr;
            }
        }
    }
}

bool UPhysicsGrabComponent::CanGrabObject(AActor* Actor, UPrimitiveComponent* Component) const
{
    if (!Actor || !Component)
        return false;

    if (!Component->IsSimulatingPhysics())
        return false;

    float ObjectMass = Component->GetMass();
    if (ObjectMass > MaxGrabMass)
        return false;

    if (Component->Mobility != EComponentMobility::Movable)
        return false;

    return true;
}

FVector UPhysicsGrabComponent::GetGrabTargetLocation() const
{
    UCameraComponent* Camera = GetPlayerCamera();
    if (!Camera)
        return FVector::ZeroVector;

    FVector CameraLocation = Camera->GetComponentLocation();
    FVector CameraForward = Camera->GetForwardVector();
    
    FVector TargetLocation = CameraLocation + (CameraForward * GrabDistance);
    
    return TargetLocation;
}

void UPhysicsGrabComponent::ApplyGrabForce(float DeltaTime)
{
    if (!GrabbedComponent)
        return;

    FVector CurrentLocation = GrabbedComponent->GetComponentLocation();
    FVector TargetLocation = GetGrabTargetLocation();
    
    FVector ForceDirection = TargetLocation - CurrentLocation;
    float Distance = ForceDirection.Size();
    
    if (Distance > 0.1f)
    {
        ForceDirection.Normalize();
        
        float ForceMagnitude = FMath::Clamp(Distance * GrabForce, 0.0f, GrabForce * 2.0f);
        FVector Force = ForceDirection * ForceMagnitude;
        
        FVector Velocity = GrabbedComponent->GetPhysicsLinearVelocity();
        FVector DampingForce = -Velocity * (GrabForce * 0.1f);
        
        GrabbedComponent->AddForce(Force + DampingForce);
    }
}

UCameraComponent* UPhysicsGrabComponent::GetPlayerCamera() const
{
    if (ACharacter* PlayerCharacter = Cast<ACharacter>(GetOwner()))
    {
        return PlayerCharacter->FindComponentByClass<UCameraComponent>();
    }
    return nullptr;
}

APlayerController* UPhysicsGrabComponent::GetPlayerController() const
{
    if (APawn* Pawn = Cast<APawn>(GetOwner()))
    {
        return Cast<APlayerController>(Pawn->GetController());
    }
    return nullptr;
}