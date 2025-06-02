#include "PuzzleManager.h"
#include "P_FixableMachine.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SpotLightComponent.h"
#include "Engine/SpotLight.h"

APuzzleManager::APuzzleManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void APuzzleManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeHUD();
    RegisterMachines();
    UpdateSpotLights();
}

void APuzzleManager::InitializeHUD()
{
    if (HUDWidgetClass)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC)
        {
            HUDWidget = CreateWidget<UUserWidget>(PC, HUDWidgetClass);
            if (HUDWidget)
            {
                HUDWidget->AddToViewport();

                ProgressBar = Cast<UProgressBar>(HUDWidget->GetWidgetFromName(TEXT("ProgressBar")));
                InteractLabel = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("InteractLabel")));

                if (ProgressBar)
                {
                    ProgressBar->SetVisibility(ESlateVisibility::Hidden);
                }

                if (InteractLabel)
                {
                    InteractLabel->SetVisibility(ESlateVisibility::Hidden);
                }
            }
        }
    }
}

void APuzzleManager::RegisterMachines()
{
    for (int32 i = 0; i < PuzzleData.Num(); ++i)
    {
        if (PuzzleData[i].Machine)
        {
            PuzzleData[i].Machine->SetPuzzleManager(this);
        }
    }
}

void APuzzleManager::OnMachineStartFixing(AP_FixableMachine* Machine)
{
    if (!VerifyWidgets()) return;

    if (ProgressBar)
    {
        ProgressBar->SetVisibility(ESlateVisibility::Visible);
        ProgressBar->SetPercent(0.0f);
    }
}

void APuzzleManager::OnMachineStopFixing(AP_FixableMachine* Machine)
{
    if (!VerifyWidgets()) return;

    if (ProgressBar)
    {
        ProgressBar->SetVisibility(ESlateVisibility::Hidden);
        ProgressBar->SetPercent(0.0f);
    }
}

void APuzzleManager::OnMachineFixed(AP_FixableMachine* Machine)
{
    if (!VerifyWidgets()) return;

    int32 PuzzleIndex = FindPuzzleIndex(Machine);
    if (PuzzleIndex != -1)
    {
        PuzzleData[PuzzleIndex].CompletionPercentage = 100.0f;
        PuzzleData[PuzzleIndex].bIsCompleted = true;
        UpdateSpotLightForPuzzle(PuzzleIndex, 100.0f);
    }

    if (ProgressBar)
    {
        ProgressBar->SetVisibility(ESlateVisibility::Hidden);
    }
    
    if (InteractLabel)
    {
        InteractLabel->SetVisibility(ESlateVisibility::Hidden);
    }

    CalculateGlobalProgression();

    bool bAllFixed = true;
    for (const FPuzzleData& Puzzle : PuzzleData)
    {
        if (Puzzle.Machine && !Puzzle.Machine->IsFixed())
        {
            bAllFixed = false;
            break;
        }
    }

    if (bAllFixed)
    {
        OnAllMachinesFixed();
    }
}

void APuzzleManager::UpdateFixingProgress(float Progress)
{
    if (!VerifyWidgets()) return;

    if (ProgressBar)
    {
        ProgressBar->SetVisibility(ESlateVisibility::Visible);
        ProgressBar->SetPercent(Progress);
    }

    AP_FixableMachine* CurrentMachine = CurrentHighlightedMachine;
    if (CurrentMachine)
    {
        int32 PuzzleIndex = FindPuzzleIndex(CurrentMachine);
        if (PuzzleIndex != -1)
        {
            PuzzleData[PuzzleIndex].CompletionPercentage = Progress * 100.0f;
            UpdateSpotLightForPuzzle(PuzzleIndex, Progress * 100.0f);
            CalculateGlobalProgression();
        }
    }
}

void APuzzleManager::ShowInteractionUI(bool bShow, const FText& Text)
{
    if (!VerifyWidgets()) return;

    if (InteractLabel)
    {
        InteractLabel->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
        if (bShow)
        {
            InteractLabel->SetText(Text);
        }
    }
}

bool APuzzleManager::VerifyWidgets() const
{
    return HUDWidget && ProgressBar && InteractLabel;
}

void APuzzleManager::OnAllMachinesFixed()
{
}

void APuzzleManager::HideProgressBar()
{
    if (ProgressBar)
    {
        ProgressBar->SetVisibility(ESlateVisibility::Hidden);
    }
}

float APuzzleManager::GetPuzzleCompletionPercentage(int32 PuzzleIndex) const
{
    if (PuzzleIndex >= 0 && PuzzleIndex < PuzzleData.Num())
    {
        return PuzzleData[PuzzleIndex].CompletionPercentage;
    }
    return 0.0f;
}

float APuzzleManager::GetGlobalProgressionPercentage() const
{
    return GlobalProgressionPercentage;
}

void APuzzleManager::CalculateGlobalProgression()
{
    if (PuzzleData.Num() == 0)
    {
        GlobalProgressionPercentage = 0.0f;
        return;
    }

    float TotalCompletion = 0.0f;
    for (const FPuzzleData& Puzzle : PuzzleData)
    {
        TotalCompletion += Puzzle.CompletionPercentage;
    }

    GlobalProgressionPercentage = TotalCompletion / PuzzleData.Num();
}

void APuzzleManager::UpdateSpotLights()
{
    for (int32 i = 0; i < SpotLights.Num(); ++i)
    {
        FSpotLightData& LightData = SpotLights[i];
        if (LightData.SpotLight && LightData.AssociatedPuzzleIndex >= 0 && LightData.AssociatedPuzzleIndex < PuzzleData.Num())
        {
            float CompletionPercentage = PuzzleData[LightData.AssociatedPuzzleIndex].CompletionPercentage;
            UpdateSpotLightForPuzzle(LightData.AssociatedPuzzleIndex, CompletionPercentage);
        }
    }
}

void APuzzleManager::UpdateSpotLightForPuzzle(int32 PuzzleIndex, float CompletionPercentage)
{
    for (FSpotLightData& LightData : SpotLights)
    {
        if (LightData.AssociatedPuzzleIndex == PuzzleIndex && LightData.SpotLight)
        {
            float Alpha = CompletionPercentage / 100.0f;
            Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

            FLinearColor NewColor = FMath::Lerp(LightData.StartColor, LightData.EndColor, Alpha);
            float NewIntensity = FMath::Lerp(LightData.StartIntensity, LightData.EndIntensity, Alpha);

            USpotLightComponent* SpotLightComp = Cast<USpotLightComponent>(LightData.SpotLight->GetLightComponent());
            if (SpotLightComp)
            {
                SpotLightComp->SetLightColor(NewColor);
                SpotLightComp->SetIntensity(NewIntensity);
            }
        }
    }
}

int32 APuzzleManager::FindPuzzleIndex(AP_FixableMachine* Machine)
{
    for (int32 i = 0; i < PuzzleData.Num(); ++i)
    {
        if (PuzzleData[i].Machine == Machine)
        {
            return i;
        }
    }
    return -1;
}