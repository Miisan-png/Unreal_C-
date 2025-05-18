#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickableItem.h"
#include "ItemManager.generated.h"

// Forward declarations
class AOxygenReplenishActor;

USTRUCT(BlueprintType)
struct FPickableItemData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    TSubclassOf<APickableItem> ItemClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    FVector PickupOffset = FVector(0, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    FRotator PickupRotation = FRotator(0, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data", meta = (ClampMin = "0.1", ClampMax = "20.0"))
    float LerpSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
    float DropImpulse = 200.0f;
};

UCLASS()
class FIRSTPERSONTEST_API AItemManager : public AActor
{
    GENERATED_BODY()

public:
    AItemManager();

    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Item Management")
    void RegisterPlayerRaycast(class AMyFPSCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category = "Item Management")
    void RegisterPickupOrigin(USceneComponent* PickupPoint);

    UFUNCTION(BlueprintCallable, Category = "Item Management")
    void RegisterItemNameWidget(class UUserWidget* ItemNameUI, class UTextBlock* NameLabel);

    UFUNCTION(BlueprintCallable, Category = "Item Management")
    bool IsLookingAtItem() const;

    UFUNCTION(BlueprintCallable, Category = "Item Management")
    bool IsLookingAtReplenishActor() const;

    UFUNCTION(BlueprintCallable, Category = "Item Management")
    bool IsHoldingItem() const { return HeldItem != nullptr; }

    UFUNCTION(BlueprintCallable, Category = "Item Management")
    APickableItem* GetHeldItem() const;

    UFUNCTION(BlueprintCallable, Category = "Item Management")
    bool PickupItem();

    UFUNCTION(BlueprintCallable, Category = "Item Management")
    void DropItem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Management")
    TArray<FPickableItemData> PickableItems;

    UPROPERTY()
    class AMyFPSCharacter* PlayerRef;

    UPROPERTY()
    USceneComponent* PickupOriginPoint;

    UPROPERTY()
    class UUserWidget* ItemNameWidget;

    UPROPERTY()
    class UTextBlock* ItemNameLabel;

    UPROPERTY()
    APickableItem* HighlightedItem;

    UPROPERTY()
    AOxygenReplenishActor* HighlightedReplenishActor;

    UPROPERTY()
    APickableItem* HeldItem;

    enum class EPickupState
    {
        None,
        PickingUp,
        Holding,
        Dropping
    };
    EPickupState CurrentPickupState = EPickupState::None;

    int32 CurrentItemIndex = -1;

    FVector TargetPosition;
    FRotator TargetRotation;

    FVector DropStartPosition;
    FRotator DropStartRotation;

    float LerpTimer = 0.0f;
    float DropLerpDuration = 0.3f;

    void ProcessRaycast();

    void UpdateHeldItem(float DeltaTime);

    int32 FindItemDataIndex(APickableItem* Item);

    void UpdateItemNameUI(const FText& DisplayName, bool bShow);
};