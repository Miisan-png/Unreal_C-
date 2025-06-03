#include "PhysicsObject.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

APhysicsObject::APhysicsObject()
{
    PrimaryActorTick.bCanEverTick = false;

    ObjectMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObjectMesh"));
    RootComponent = ObjectMesh;
    ObjectMesh->SetSimulatePhysics(true);
    ObjectMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    ObjectMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
    ObjectMesh->SetLinearDamping(1.0f);
    ObjectMesh->SetAngularDamping(1.0f);
    ObjectMesh->SetUseCCD(true); 
    ObjectMesh->SetNotifyRigidBodyCollision(true);
}

void APhysicsObject::BeginPlay()
{
    Super::BeginPlay();
    
    if (ObjectMesh && ObjectMesh->GetMaterial(0))
    {
        OriginalMaterial = ObjectMesh->GetMaterial(0);
    }
    
    if (ObjectMesh)
    {
        ObjectMesh->SetMassOverrideInKg(NAME_None, ObjectMass);
    }
    
    if (ObjectMesh)
    {
        ObjectMesh->OnComponentHit.AddDynamic(this, &APhysicsObject::OnHit);
    }
}

void APhysicsObject::Highlight(bool bHighlight)
{
    if (!ObjectMesh)
        return;

    if (bHighlight && HighlightMaterial)
    {
        ObjectMesh->SetMaterial(0, HighlightMaterial);
    }
    else if (!bHighlight && OriginalMaterial)
    {
        ObjectMesh->SetMaterial(0, OriginalMaterial);
    }
}

void APhysicsObject::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!CollisionSound)
        return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastCollisionSoundTime < CollisionSoundCooldown)
        return;
    
    float ImpactVelocity = NormalImpulse.Size();
    if (ImpactVelocity < MinCollisionVelocity)
        return;
    
    if (OtherActor && OtherActor->IsA(APawn::StaticClass()))
        return;
    
    UGameplayStatics::PlaySoundAtLocation(
        GetWorld(),
        CollisionSound,
        GetActorLocation(),
        FMath::Clamp(ImpactVelocity / 1000.0f, 0.1f, 1.0f)
    );
    
    LastCollisionSoundTime = CurrentTime;
}