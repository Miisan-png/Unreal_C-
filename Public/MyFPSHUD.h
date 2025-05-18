// MyFPSHUD.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MyFPSHUD.generated.h"

/**
 *
 */
UCLASS()
class FIRSTPERSONTEST_API AMyFPSHUD : public AHUD
{
    GENERATED_BODY()

public:
    AMyFPSHUD();

    // Draw HUD
    virtual void DrawHUD() override;

    // Set if we're looking at an interactable
    void SetLookingAtInteractable(bool bInteractable) { bLookingAtInteractable = bInteractable; }

private:
    // Crosshair texture
    UPROPERTY(EditDefaultsOnly, Category = "Crosshair")
    UTexture2D* CrosshairTexture;

    // Crosshair properties
    UPROPERTY(EditAnywhere, Category = "Crosshair")
    bool bShowCrosshair = true;

    UPROPERTY(EditAnywhere, Category = "Crosshair", meta = (ClampMin = "1.0", ClampMax = "100.0"))
    float CrosshairSize = 8.0f;

    UPROPERTY(EditAnywhere, Category = "Crosshair")
    FLinearColor CrosshairColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, Category = "Crosshair")
    FLinearColor CrosshairHighlightColor = FLinearColor::Yellow;

    // Flag to track if looking at interactable items
    bool bLookingAtInteractable = false;
};