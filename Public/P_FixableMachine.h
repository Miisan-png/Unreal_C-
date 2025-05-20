#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IInteractable.h"
#include "P_FixableMachine.generated.h"

class UStaticMeshComponent;
class UMaterialInterface;
class APuzzleManager;

UCLASS()
class FIRSTPERSONTEST_API AP_FixableMachine : public AActor, public IInteractable
{
    GENERATED_BODY()

public:
    AP_FixableMachine();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // IInteractable Interface Implementation
    virtual void OnHighlight_Implementation() override;
    virtual void OnUnhighlight_Implementation() override;
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual bool CanInteract_Implementation() const override;
    virtual FText GetInteractionText_Implementation() const override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MachineMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Machine")
    UMaterialInterface* BrokenMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Machine")
    UMaterialInterface* FixedMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Machine")
    UMaterialInterface* HighlightMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Machine")
    float TimeToFix = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Machine")
    FText InteractionText = FText::FromString("Hold [E] to Fix");

    UFUNCTION(BlueprintCallable, Category = "Machine")
    void StartFixing();

    UFUNCTION(BlueprintCallable, Category = "Machine")
    void StopFixing();

    UFUNCTION(BlueprintCallable, Category = "Machine")
    void SetPuzzleManager(APuzzleManager* Manager);

    UFUNCTION(BlueprintPure, Category = "Machine")
    bool IsFixed() const { return bIsFixed; }

    UFUNCTION(BlueprintPure, Category = "Machine")
    bool IsBeingFixed() const { return bIsBeingFixed; }

protected:
    UPROPERTY()
    APuzzleManager* PuzzleManagerRef;

    UPROPERTY()
    UMaterialInterface* OriginalMaterial;

    bool bIsFixed;
    bool bIsBeingFixed;
    float FixingProgress;

    void UpdateFixingProgress(float DeltaTime);
    void CompleteFix();
};