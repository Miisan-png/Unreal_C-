#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickableItem.generated.h"

UCLASS()
class FIRSTPERSONTEST_API APickableItem : public AActor
{
    GENERATED_BODY()
    
public:    
    APickableItem();
    
    virtual void Tick(float DeltaTime) override;
    
    // Called when the item is picked up
    void PickUp(AActor* NewOwner, USceneComponent* AttachTo);
    
    // Called when the item is dropped
    void Drop(FVector DropImpulse = FVector::ZeroVector);
    
    // Check if the item is currently being held
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool IsHeld() const { return bIsHeld; }
    
    // Get the mesh component
    UFUNCTION(BlueprintCallable, Category = "Components")
    UStaticMeshComponent* GetMesh() const { return ItemMesh; }

    // Get the display name of the item
    UFUNCTION(BlueprintCallable, Category = "Item Properties")
    FText GetDisplayName() const { return DisplayName; }

    // Set the display name of the item
    UFUNCTION(BlueprintCallable, Category = "Item Properties")
    void SetDisplayName(const FText& NewDisplayName) { DisplayName = NewDisplayName; }

protected:
    virtual void BeginPlay() override;

    // Mesh component for the item
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ItemMesh;

    // Display name for the item (shown in UI)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties")
    FText DisplayName;
    
    // Highlight material when looking at the item
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
    UMaterialInterface* HighlightMaterial;
    
    // Original material to revert to when not highlighted
    UPROPERTY()
    UMaterialInterface* OriginalMaterial;
    
    // Is this item currently held by the player
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
    bool bIsHeld;
    
    // The actor that picked up this item
    UPROPERTY()
    AActor* ItemOwner;
    
    // The component the item is attached to when picked up
    UPROPERTY()
    USceneComponent* AttachComponent;
    
    // Target transform for smooth pickup lerping
    UPROPERTY()
    FTransform TargetTransform;

    // Current lerp alpha for smooth pickup
    UPROPERTY()
    float LerpAlpha;

    // Whether we're currently lerping to pickup position
    UPROPERTY()
    bool bIsLerping;

    // Pickup lerp speed (higher = faster)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float PickupLerpSpeed;

    // Previous owner location for sway calculation
    UPROPERTY()
    FVector PreviousOwnerLocation;

    // How much the held item sways when moving (higher = more sway)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MovementSwayAmount;

    // Smoothing factor for sway (higher = smoother)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "1.0", ClampMax = "20.0"))
    float SwaySmoothing;

    // Current sway offset
    UPROPERTY()
    FVector CurrentSwayOffset;

public:
    virtual void Highlight(bool bHighlight);
};