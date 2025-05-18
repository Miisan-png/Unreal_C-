#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "ItemNameWidget.generated.h"

UCLASS()
class FIRSTPERSONTEST_API UItemNameWidget : public UUserWidget {
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Item UI")
    UTextBlock* GetItemNameLabel() const { return item_name_label; }

protected:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* item_name_label;
    int32 FrameCount;
};