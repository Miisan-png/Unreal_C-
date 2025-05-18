#include "MyFPSHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"
#include "CanvasItem.h"
#include "UObject/ConstructorHelpers.h"

AMyFPSHUD::AMyFPSHUD()
{
    static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshairTexObj(TEXT("/Engine/EngineResources/WhiteSquareTexture"));
    if (CrosshairTexObj.Succeeded())
    {
        CrosshairTexture = CrosshairTexObj.Object;
    }
}

void AMyFPSHUD::DrawHUD()
{
    Super::DrawHUD();
    
    if (!bShowCrosshair)
        return;
    
    if (Canvas && CrosshairTexture)
    {
        FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);
        
        FVector2D CrosshairDrawPosition(Center.X - (CrosshairSize * 0.5f), Center.Y - (CrosshairSize * 0.5f));
        
        FLinearColor FinalColor = bLookingAtInteractable ? CrosshairHighlightColor : CrosshairColor;
        FCanvasTileItem TileItem(CrosshairDrawPosition, CrosshairTexture->Resource, FVector2D(CrosshairSize, CrosshairSize), FinalColor);
        TileItem.BlendMode = SE_BLEND_Translucent;
        Canvas->DrawItem(TileItem);
    }
}