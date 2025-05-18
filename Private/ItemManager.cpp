#include "ItemManager.h"
#include "MyFPSCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"

AItemManager::AItemManager()
{
    PrimaryActorTick.bCanEverTick = true;

    PlayerRef = nullptr;
    PickupOriginPoint = nullptr;
    ItemNameWidget = nullptr;
    ItemNameLabel = nullptr;
    HighlightedItem = nullptr;
    HeldItem = nullptr;
    CurrentPickupState = EPickupState::None;
    CurrentItemIndex = -1;
    LerpTimer = 0.0f;
}

void AItemManager::BeginPlay()
{
    Super::BeginPlay();

    if (ItemNameWidget)
    {
        ItemNameWidget->SetVisibility(ESlateVisibility::Hidden);
    }
}

void AItemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ProcessRaycast();

    if (HeldItem)
    {
        UpdateHeldItem(DeltaTime);
    }
}

void AItemManager::RegisterPlayerRaycast(AMyFPSCharacter* PlayerCharacter)
{
    PlayerRef = PlayerCharacter;
}

void AItemManager::RegisterPickupOrigin(USceneComponent* PickupPoint)
{
    PickupOriginPoint = PickupPoint;
}

void AItemManager::RegisterItemNameWidget(UUserWidget* ItemNameUI, UTextBlock* NameLabel)
{
    UE_LOG(LogTemp, Warning, TEXT("RegisterItemNameWidget called"));

    if (!ItemNameUI)
    {
        UE_LOG(LogTemp, Error, TEXT("RegisterItemNameWidget: ItemNameUI is NULL!"));
        return;
    }

    if (!NameLabel)
    {
        UE_LOG(LogTemp, Error, TEXT("RegisterItemNameWidget: NameLabel is NULL!"));
        return;
    }

    this->ItemNameWidget = ItemNameUI;
    this->ItemNameLabel = NameLabel;

    UE_LOG(LogTemp, Warning, TEXT("Item name UI widget and label successfully registered"));

    ItemNameLabel->SetText(FText::FromString("REGISTERED TEST TEXT"));

    ItemNameWidget->SetVisibility(ESlateVisibility::Visible);
    ItemNameLabel->SetVisibility(ESlateVisibility::Visible);
}

bool AItemManager::IsLookingAtItem() const
{
    return HighlightedItem != nullptr;
}

bool AItemManager::PickupItem()
{
    if (!HighlightedItem || !PickupOriginPoint || HeldItem)
        return false;

    CurrentItemIndex = FindItemDataIndex(HighlightedItem);
    if (CurrentItemIndex < 0)
        return false;

    HeldItem = HighlightedItem;

    HeldItem->SetDisplayName(PickableItems[CurrentItemIndex].DisplayName);

    UpdateItemNameUI(FText::GetEmpty(), false);

    HighlightedItem = nullptr;

    HeldItem->PickUp(PickupOriginPoint->GetOwner(), PickupOriginPoint);

    DropStartPosition = HeldItem->GetActorLocation();
    DropStartRotation = HeldItem->GetActorRotation();

    LerpTimer = 0.0f;

    CurrentPickupState = EPickupState::PickingUp;

    return true;
}

void AItemManager::DropItem()
{
    if (HeldItem && CurrentPickupState != EPickupState::Dropping)
    {
        DropStartPosition = HeldItem->GetActorLocation();
        DropStartRotation = HeldItem->GetActorRotation();

        if (PlayerRef)
        {
            FVector ForwardVector = PlayerRef->GetActorForwardVector();
            TargetPosition = HeldItem->GetActorLocation() + ForwardVector * 50.0f;

            TargetRotation = HeldItem->GetActorRotation();
            TargetRotation.Pitch += 10.0f;
        }

        LerpTimer = 0.0f;
        CurrentPickupState = EPickupState::Dropping;
    }
}

void AItemManager::ProcessRaycast()
{
    if (!PlayerRef)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProcessRaycast: PlayerRef is NULL"));
        return;
    }

    UE_LOG(LogTemp, Display, TEXT("ProcessRaycast running..."));

    APlayerController* PC = Cast<APlayerController>(PlayerRef->GetController());
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProcessRaycast: PlayerController is NULL"));
        return;
    }

    FVector CameraLocation;
    FRotator CameraRotation;
    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

    FVector CameraForward = CameraRotation.Vector();
    FVector RayEnd = CameraLocation + (CameraForward * PlayerRef->GetRaycastDistance());

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(PlayerRef);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        CameraLocation,
        RayEnd,
        ECC_Visibility,
        QueryParams
    );

    if (bHit && HitResult.GetActor())
    {
        UE_LOG(LogTemp, Display, TEXT("Raycast hit: %s"), *HitResult.GetActor()->GetName());
    }

    if (HeldItem)
    {
        if (HighlightedItem)
        {
            HighlightedItem->Highlight(false);
            HighlightedItem = nullptr;

            UpdateItemNameUI(FText::GetEmpty(), false);
            UE_LOG(LogTemp, Display, TEXT("Holding item, hiding UI"));
        }
        return;
    }

    APickableItem* HitItem = nullptr;
    if (bHit && HitResult.GetActor())
    {
        HitItem = Cast<APickableItem>(HitResult.GetActor());
        if (HitItem)
        {
            UE_LOG(LogTemp, Warning, TEXT("Hit a pickable item: %s"), *HitItem->GetName());
        }
    }

    if (HighlightedItem && HighlightedItem != HitItem)
    {
        UE_LOG(LogTemp, Display, TEXT("No longer looking at highlighted item"));
        HighlightedItem->Highlight(false);
        HighlightedItem = nullptr;

        UpdateItemNameUI(FText::GetEmpty(), false);
    }

    if (HitItem && HitItem != HighlightedItem && !HitItem->IsHeld())
    {
        UE_LOG(LogTemp, Warning, TEXT("Looking at new pickable item"));
        HighlightedItem = HitItem;
        HighlightedItem->Highlight(true);

        int32 ItemIndex = FindItemDataIndex(HighlightedItem);
        if (ItemIndex >= 0)
        {
            FText DisplayName = PickableItems[ItemIndex].DisplayName;
            UE_LOG(LogTemp, Warning, TEXT("Item data found! Display name: %s"), *DisplayName.ToString());

            HighlightedItem->SetDisplayName(DisplayName);

            UpdateItemNameUI(DisplayName, true);
        }
        else
        {
            FText ItemName = HighlightedItem->GetDisplayName();
            UE_LOG(LogTemp, Warning, TEXT("Using item's own name: %s"), *ItemName.ToString());
            UpdateItemNameUI(ItemName, true);
        }
    }
}

void AItemManager::UpdateItemNameUI(const FText& DisplayName, bool bShow)
{
    UE_LOG(LogTemp, Warning, TEXT("UpdateItemNameUI called - Show: %s, Text: %s"),
        bShow ? TEXT("true") : TEXT("false"),
        *DisplayName.ToString());

    if (!ItemNameWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("ItemNameWidget is NULL - UI not registered!"));
        return;
    }

    if (!ItemNameLabel)
    {
        UE_LOG(LogTemp, Error, TEXT("ItemNameLabel is NULL - Text block not registered!"));
        return;
    }

    ItemNameLabel->SetText(DisplayName);
    UE_LOG(LogTemp, Warning, TEXT("Set text on ItemNameLabel to: %s"), *DisplayName.ToString());

    ESlateVisibility NewVisibility = bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
    ItemNameWidget->SetVisibility(NewVisibility);
    UE_LOG(LogTemp, Warning, TEXT("Set widget visibility to: %s"),
        (NewVisibility == ESlateVisibility::Visible) ? TEXT("Visible") : TEXT("Hidden"));

    ItemNameLabel->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    UE_LOG(LogTemp, Warning, TEXT("Set text visibility to: %s"),
        bShow ? TEXT("Visible") : TEXT("Hidden"));
}

void AItemManager::UpdateHeldItem(float DeltaTime)
{
    if (!HeldItem || CurrentItemIndex < 0 || CurrentItemIndex >= PickableItems.Num() || !PickupOriginPoint)
        return;

    const FPickableItemData& ItemData = PickableItems[CurrentItemIndex];

    switch (CurrentPickupState)
    {
        case EPickupState::PickingUp:
        {
            LerpTimer += DeltaTime;
            float Alpha = FMath::Clamp(LerpTimer * ItemData.LerpSpeed, 0.0f, 1.0f);

            FVector TargetPos = PickupOriginPoint->GetComponentLocation() +
                              PickupOriginPoint->GetComponentRotation().RotateVector(ItemData.PickupOffset);
            FRotator TargetRot = PickupOriginPoint->GetComponentRotation() + ItemData.PickupRotation;

            FVector NewLocation = FMath::Lerp(DropStartPosition, TargetPos, Alpha);
            FRotator NewRotation = FMath::Lerp(DropStartRotation, TargetRot, Alpha);

            HeldItem->SetActorLocationAndRotation(NewLocation, NewRotation);

            if (Alpha >= 1.0f)
            {
                CurrentPickupState = EPickupState::Holding;
            }
            break;
        }

        case EPickupState::Holding:
        {
            FVector TargetPos = PickupOriginPoint->GetComponentLocation() +
                              PickupOriginPoint->GetComponentRotation().RotateVector(ItemData.PickupOffset);
            FRotator TargetRot = PickupOriginPoint->GetComponentRotation() + ItemData.PickupRotation;

            FVector NewLocation = FMath::VInterpTo(HeldItem->GetActorLocation(), TargetPos, DeltaTime, ItemData.LerpSpeed);
            FRotator NewRotation = FMath::RInterpTo(HeldItem->GetActorRotation(), TargetRot, DeltaTime, ItemData.LerpSpeed);

            HeldItem->SetActorLocationAndRotation(NewLocation, NewRotation);
            break;
        }

        case EPickupState::Dropping:
        {
            LerpTimer += DeltaTime;
            float Alpha = FMath::Clamp(LerpTimer / DropLerpDuration, 0.0f, 1.0f);

            FVector NewLocation = FMath::Lerp(DropStartPosition, TargetPosition, Alpha);
            FRotator NewRotation = FMath::Lerp(DropStartRotation, TargetRotation, Alpha);

            HeldItem->SetActorLocationAndRotation(NewLocation, NewRotation);

            if (Alpha >= 1.0f)
            {
                if (HeldItem && PlayerRef)
                {
                    FVector ForwardVector = PlayerRef->GetActorForwardVector();
                    HeldItem->Drop(ForwardVector * ItemData.DropImpulse);
                }
                else
                {
                    HeldItem->Drop();
                }

                HeldItem = nullptr;
                CurrentItemIndex = -1;
                CurrentPickupState = EPickupState::None;
            }
            break;
        }

        default:
            break;
    }
}

int32 AItemManager::FindItemDataIndex(APickableItem* Item)
{
    if (!Item)
        return -1;

    for (int32 i = 0; i < PickableItems.Num(); ++i)
    {
        if (Item->IsA(PickableItems[i].ItemClass))
        {
            return i;
        }
    }

    return -1;
}

APickableItem* AItemManager::GetHeldItem() const
{
    return HeldItem;
}