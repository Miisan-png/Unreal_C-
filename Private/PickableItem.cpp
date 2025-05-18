#include "PickableItem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

APickableItem::APickableItem()
{
    PrimaryActorTick.bCanEverTick = true;

    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    SetRootComponent(ItemMesh);

    ItemMesh->SetSimulatePhysics(true);
    ItemMesh->SetCollisionProfileName(TEXT("PhysicsActor"));

    bIsHeld = false;
    bIsLerping = false;
    LerpAlpha = 0.0f;
    ItemOwner = nullptr;
    AttachComponent = nullptr;

    PickupLerpSpeed = 5.0f;
    MovementSwayAmount = 0.3f;
    SwaySmoothing = 5.0f;

    CurrentSwayOffset = FVector::ZeroVector;
    PreviousOwnerLocation = FVector::ZeroVector;

    DisplayName = FText::FromString(GetName());
}

void APickableItem::BeginPlay()
{
    Super::BeginPlay();

    if (ItemMesh->GetMaterial(0))
    {
        OriginalMaterial = ItemMesh->GetMaterial(0);
    }

    if (DisplayName.IsEmpty())
    {
        DisplayName = FText::FromString(GetName());
    }
}

void APickableItem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (bIsLerping && bIsHeld)
    {
        LerpAlpha = FMath::Clamp(LerpAlpha + (DeltaTime * PickupLerpSpeed), 0.0f, 1.0f);
        float EasedAlpha = 1.0f - FMath::Pow(1.0f - LerpAlpha, 3.0f);
        FTransform CurrentTransform = GetActorTransform();
        FTransform NewTransform;
        NewTransform.SetLocation(FMath::Lerp(CurrentTransform.GetLocation(), TargetTransform.GetLocation(), EasedAlpha));
        NewTransform.SetRotation(FMath::Lerp(CurrentTransform.GetRotation(), TargetTransform.GetRotation(), EasedAlpha));
        SetActorTransform(NewTransform);
        if (LerpAlpha >= 1.0f)
        {
            bIsLerping = false;
            AttachToComponent(AttachComponent, FAttachmentTransformRules::KeepWorldTransform);
        }
    }
    if (bIsHeld && !bIsLerping && ItemOwner)
    {
        FVector CurrentOwnerLocation = ItemOwner->GetActorLocation();
        FVector OwnerVelocity = (CurrentOwnerLocation - PreviousOwnerLocation) / DeltaTime;
        FVector DesiredSway = FVector(
            -OwnerVelocity.Y * MovementSwayAmount * 0.01f,  
            OwnerVelocity.X * MovementSwayAmount * 0.01f,   
            0.0f                                         
        );
        CurrentSwayOffset = FMath::Lerp(
            CurrentSwayOffset,
            DesiredSway,
            DeltaTime * SwaySmoothing
        );

        if (!CurrentSwayOffset.IsNearlyZero() && AttachComponent)
        {
            FVector RelativeLocation = GetActorLocation() - AttachComponent->GetComponentLocation();
            FVector NewRelativeLocation = RelativeLocation + CurrentSwayOffset;
            SetActorLocation(AttachComponent->GetComponentLocation() + NewRelativeLocation);
        }

        PreviousOwnerLocation = CurrentOwnerLocation;
    }
}

void APickableItem::PickUp(AActor* NewOwner, USceneComponent* AttachTo)
{
    Highlight(false);
    ItemOwner = NewOwner;
    AttachComponent = AttachTo;

    TargetTransform = AttachTo->GetComponentTransform();
    if (ItemOwner)
    {
        PreviousOwnerLocation = ItemOwner->GetActorLocation();
    }

    ItemMesh->SetSimulatePhysics(false);
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    bIsLerping = true;
    LerpAlpha = 0.0f;

    bIsHeld = true;
}

void APickableItem::Drop(FVector DropImpulse)
{
    if (!bIsHeld)
        return;

    if (bIsLerping)
    {
        bIsLerping = false;
    }
    else
    {
        DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    }

    ItemMesh->SetSimulatePhysics(true);
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    if (!DropImpulse.IsZero())
    {
        ItemMesh->AddImpulse(DropImpulse);
    }
    else if (ItemOwner)
    {
        FVector DownVector = FVector(0, 0, -1);
        ItemMesh->AddImpulse(DownVector * 50.0f);
    }

    CurrentSwayOffset = FVector::ZeroVector;

    ItemOwner = nullptr;
    AttachComponent = nullptr;

    bIsHeld = false;
}

void APickableItem::Highlight(bool bHighlight)
{
    if (bHighlight && HighlightMaterial)
    {
        ItemMesh->SetMaterial(0, HighlightMaterial);
    }
    else if (!bHighlight && OriginalMaterial)
    {
        ItemMesh->SetMaterial(0, OriginalMaterial);
    }
}