#include "ItemNameWidget.h"

void UItemNameWidget::NativeConstruct() 
{
    Super::NativeConstruct();
    
    UE_LOG(LogTemp, Warning, TEXT("ItemNameWidget constructed"));
    
    SetVisibility(ESlateVisibility::Visible);
    
    if (item_name_label)
    {
        UE_LOG(LogTemp, Warning, TEXT("item_name_label found in widget"));
        
        item_name_label->SetVisibility(ESlateVisibility::Visible);
        item_name_label->SetText(FText::GetEmpty()); 
        
        UE_LOG(LogTemp, Warning, TEXT("Text set to empty"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ERROR: item_name_label NOT found - check widget design!"));
    }
    
    FrameCount = 0;
}

void UItemNameWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    
    if (FrameCount < 5 && item_name_label)
    {
        item_name_label->SetText(FText::GetEmpty());
        FrameCount++;
        
        if (FrameCount == 1) 
        {
            UE_LOG(LogTemp, Warning, TEXT("Clearing text in NativeTick, frame %d"), FrameCount);
        }
    }
}