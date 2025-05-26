// Fill out your copyright notice in the Description page of Project Settings.

#include "GuideTrail.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"

AGuideTrail::AGuideTrail()
{
    PrimaryActorTick.bCanEverTick = true;

    SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
    RootComponent = SplineComponent;
    SplineComponent->SetupAttachment(RootComponent);
    
    NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
    NiagaraComponent->SetupAttachment(RootComponent);
    NiagaraComponent->SetAutoActivate(false);
}

void AGuideTrail::BeginPlay()
{
    Super::BeginPlay();
    
    StartLocation = GetActorLocation();
    
    if (TrailEffect)
    {
        NiagaraComponent->SetAsset(TrailEffect);
    }
    
    if (bAutoActivate)
    {
        ActivateTrail();
    }
    else
    {
        DeactivateTrail();
    }
}

void AGuideTrail::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsActive && TargetActor)
    {
        TargetLocation = TargetActor->GetActorLocation();
        UpdateTrail(DeltaTime);
    }
}

void AGuideTrail::ActivateTrail()
{
    if (!bIsActive)
    {
        bIsActive = true;
        ElapsedTime = 0.0f;
        StartLocation = GetActorLocation();
        CurrentTrailLocation = StartLocation;
        
        if (TargetActor)
        {
            TargetLocation = TargetActor->GetActorLocation();
        }
        
        SplineComponent->ClearSplinePoints(false);
        SplineComponent->AddSplinePoint(StartLocation, ESplineCoordinateSpace::World, false);
        SplineComponent->AddSplinePoint(StartLocation, ESplineCoordinateSpace::World, false);
        SplineComponent->UpdateSpline();
        
        NiagaraComponent->SetVisibility(true);
        NiagaraComponent->Activate(true);
    }
}

void AGuideTrail::DeactivateTrail()
{
    if (bIsActive)
    {
        bIsActive = false;
        NiagaraComponent->Deactivate();
        NiagaraComponent->SetVisibility(false);
        ClearTrail();
    }
}

void AGuideTrail::UpdateTrail(float DeltaTime)
{
    ElapsedTime += DeltaTime;
    
    if (ElapsedTime >= TrailLifetime)
    {
        DeactivateTrail();
        return;
    }
    
    float Alpha = FMath::Clamp(ElapsedTime / TrailLifetime, 0.0f, 1.0f);
    FVector Direction = (TargetLocation - StartLocation).GetSafeNormal();
    float Distance = FMath::Min(TrailSpeed * ElapsedTime, FVector::Dist(StartLocation, TargetLocation));
    
    CurrentTrailLocation = StartLocation + Direction * Distance;
    
    SplineComponent->SetLocationAtSplinePoint(1, CurrentTrailLocation, ESplineCoordinateSpace::World, true);
    
    NiagaraComponent->SetWorldLocation(CurrentTrailLocation);
    
    UpdateSplineMeshes();
}

void AGuideTrail::ClearTrail()
{
    for (auto& Mesh : SplineMeshes)
    {
        if (Mesh)
        {
            Mesh->DestroyComponent();
        }
    }
    
    SplineMeshes.Empty();
    SplineComponent->ClearSplinePoints(false);
}

void AGuideTrail::UpdateSplineMeshes()
{
    ClearTrail();
    
    if (!TrailMesh)
    {
        return;
    }
    
    SplineComponent->ClearSplinePoints(false);
    SplineComponent->AddSplinePoint(StartLocation, ESplineCoordinateSpace::World, false);
    SplineComponent->AddSplinePoint(CurrentTrailLocation, ESplineCoordinateSpace::World, false);
    SplineComponent->UpdateSpline();
    
    FVector StartPos, StartTangent, EndPos, EndTangent;
    SplineComponent->GetLocationAndTangentAtSplinePoint(0, StartPos, StartTangent, ESplineCoordinateSpace::World);
    SplineComponent->GetLocationAndTangentAtSplinePoint(1, EndPos, EndTangent, ESplineCoordinateSpace::World);
    
    USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
    SplineMesh->SetMobility(EComponentMobility::Movable);
    SplineMesh->SetStaticMesh(TrailMesh);
    SplineMesh->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent, true);
    SplineMesh->SetStartScale(FVector2D(TrailWidth, TrailWidth));
    SplineMesh->SetEndScale(FVector2D(TrailWidth, TrailWidth));
    SplineMesh->RegisterComponent();
    
    UMaterialInstanceDynamic* DynamicMaterial = SplineMesh->CreateAndSetMaterialInstanceDynamic(0);
    if (DynamicMaterial)
    {
        DynamicMaterial->SetVectorParameterValue(TEXT("Color"), TrailColor);
    }
    
    SplineMeshes.Add(SplineMesh);
}