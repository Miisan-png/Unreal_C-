// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "NiagaraComponent.h"
#include "GuideTrail.generated.h"

UCLASS()
class FIRSTPERSONTEST_API AGuideTrail : public AActor
{
    GENERATED_BODY()
    
public:    
    AGuideTrail();
    virtual void Tick(float DeltaTime) override;
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
    AActor* TargetActor;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
    UStaticMesh* TrailMesh;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
    UNiagaraSystem* TrailEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
    float TrailLifetime = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
    float TrailSpeed = 1000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
    float TrailWidth = 20.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
    FLinearColor TrailColor = FLinearColor(0.0f, 1.0f, 0.8f, 1.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
    bool bAutoActivate = true;
    
    UFUNCTION(BlueprintCallable, Category = "Trail")
    void ActivateTrail();
    
    UFUNCTION(BlueprintCallable, Category = "Trail")
    void DeactivateTrail();

private:
    UPROPERTY()
    USplineComponent* SplineComponent;
    
    UPROPERTY()
    UNiagaraComponent* NiagaraComponent;
    
    UPROPERTY()
    TArray<USplineMeshComponent*> SplineMeshes;
    
    UPROPERTY()
    float ElapsedTime = 0.0f;
    
    UPROPERTY()
    bool bIsActive = false;
    
    UPROPERTY()
    FVector StartLocation;
    
    UPROPERTY()
    FVector TargetLocation;
    
    UPROPERTY()
    FVector CurrentTrailLocation;
    
    void UpdateTrail(float DeltaTime);
    void ClearTrail();
    void UpdateSplineMeshes();
};