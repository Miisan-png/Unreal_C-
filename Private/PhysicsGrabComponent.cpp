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

    // Update highlighting
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

    // Apply physics forces to move the object
    ApplyGrabForce(DeltaTime);

    // Debug visualization
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

    // Perform trace to get the exact hit point
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
        
        // Calculate grab offset from object center
        FVector ObjectCenter = GrabbedComponent->GetComponentLocation();
        GrabOffset = HitResult.ImpactPoint - ObjectCenter;
        
        bIsGrabbing = true;
        CurrentRotationPitch = 0.0f;
        CurrentRotationYaw = 0.0f;

        // Play grab sound
        if (GrabSound)
        {
            UGameplayStatics::PlaySound2D(GetWorld(), GrabSound);
        }

        // Remove from highlighting since we're now grabbing it
        SetObjectHighlight(HighlightedActor, false);
        HighlightedActor = nullptr;

        UE_LOG(LogTemp, Warning, TEXT("Grabbed object: %s"), *GrabbedActor->GetName());
    }
}

void UPhysicsGrabComponent::StopGrab()
{
    if (!bIsGrabbing)
        return;

    bIsGrabbing = false;
    
    if (GrabbedActor)
    {
        // Play release sound
        if (ReleaseSound)
        {
            UGameplayStatics::PlaySound2D(GetWorld(), ReleaseSound);
        }

        UE_LOG(LogTemp, Warning, TEXT("Released object: %s"), *GrabbedActor->GetName());
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

    // Add player velocity for more realistic throwing
    if (ACharacter* PlayerCharacter = Cast<ACharacter>(GetOwner()))
    {
        ThrowVelocity += PlayerCharacter->GetVelocity();
    }

    // Apply impulse
    GrabbedComponent->AddImpulse(ThrowVelocity, NAME_None, true);

    // Stop grabbing
    StopGrab();
}

void UPhysicsGrabComponent::RotateGrabbedObject(float PitchInput, float YawInput)
{
    if (!bIsGrabbing || !GrabbedComponent)
        return;

    CurrentRotationPitch += PitchInput * RotationSpeed * GetWorld()->GetDeltaSeconds();
    CurrentRotationYaw += YawInput * RotationSpeed * GetWorld()->GetDeltaSeconds();

    // Apply rotation as angular velocity
    FVector AngularVelocity = FVector(CurrentRotationPitch, CurrentRotationYaw, 0.0f);
    GrabbedComponent->SetPhysicsAngularVelocityInDegrees(AngularVelocity);
}

void UPhysicsGrabComponent::SetObjectHighlight(AActor* Actor, bool bHighlight)
{
    if (!Actor)
        return;

    // Try to highlight PickableItem first
    if (APickableItem* PickableItem = Cast<APickableItem>(Actor))
    {
        PickableItem->Highlight(bHighlight);
        return;
    }

    // Fallback to manual material highlighting
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

    // Must have physics simulation enabled
    if (!Component->IsSimulatingPhysics())
        return false;

    // Check mass limit
    float ObjectMass = Component->GetMass();
    if (ObjectMass > MaxGrabMass)
        return false;

    // Must be movable
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
    
    // Target position in front of camera, adjusted by grab offset
    FVector TargetLocation = CameraLocation + (CameraForward * GrabDistance);
    
    return TargetLocation;
}

void UPhysicsGrabComponent::ApplyGrabForce(float DeltaTime)
{
    if (!GrabbedComponent)
        return;

    FVector CurrentLocation = GrabbedComponent->GetComponentLocation();
    FVector TargetLocation = GetGrabTargetLocation();
    
    // Calculate force direction and magnitude
    FVector ForceDirection = TargetLocation - CurrentLocation;
    float Distance = ForceDirection.Size();
    
    if (Distance > 0.1f) // Avoid jittering when very close
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