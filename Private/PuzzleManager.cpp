#include "PuzzleManager.h"
#include "P_FixableMachine.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PointLightComponent.h"
#include "Engine/PointLight.h"

APuzzleManager::APuzzleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    HUDWidget = nullptr;
    PlayerHUDWidget = nullptr;
    ProgressBar = nullptr;
    InteractLabel = nullptr;
    ObjectiveLabel = nullptr;
    TaskLabel = nullptr;
    CurrentHighlightedMachine = nullptr;
}

APuzzleManager::~APuzzleManager()
{
    if (HUDWidget && IsValid(HUDWidget) && !IsEngineExitRequested())
    {
        HUDWidget->RemoveFromParent();
    }
    
    if (PlayerHUDWidget && IsValid(PlayerHUDWidget) && !IsEngineExitRequested())
    {
        PlayerHUDWidget->RemoveFromParent();
    }
    
    HUDWidget = nullptr;
    PlayerHUDWidget = nullptr;
    ProgressBar = nullptr;
    InteractLabel = nullptr;
    ObjectiveLabel = nullptr;
    TaskLabel = nullptr;
    CurrentHighlightedMachine = nullptr;
}

void APuzzleManager::BeginPlay()
{
    Super::BeginPlay();
    
    SortPuzzlesByOrder();
    InitializeHUD();
    InitializePlayerHUD();
    RegisterMachines();
    UpdatePointLights();
    
    if (PuzzleData.Num() > 0)
    {
        PuzzleData[0].bIsActive = true;
        UpdatePlayerHUDLabels();
    }
    
    for (FPointLightData& LightData : PointLights)
    {
        if (LightData.PointLight && IsValid(LightData.PointLight))
        {
            LightData.CurrentColor = LightData.StartColor;
            LightData.CurrentIntensity = LightData.StartIntensity;
            LightData.TargetColor = LightData.StartColor;
            LightData.TargetIntensity = LightData.StartIntensity;
            
            UPointLightComponent* PointLightComp = Cast<UPointLightComponent>(LightData.PointLight->GetLightComponent());
            if (PointLightComp && IsValid(PointLightComp))
            {
                PointLightComp->SetLightColor(LightData.CurrentColor);
                PointLightComp->SetIntensity(LightData.CurrentIntensity);
            }
        }
    }
}

void APuzzleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bUseInterpolation)
    {
        for (FPointLightData& LightData : PointLights)
        {
            if (LightData.PointLight && IsValid(LightData.PointLight))
            {
                UPointLightComponent* PointLightComp = Cast<UPointLightComponent>(LightData.PointLight->GetLightComponent());
                if (PointLightComp && IsValid(PointLightComp))
                {
                    LightData.CurrentColor = FLinearColor::LerpUsingHSV(LightData.CurrentColor, LightData.TargetColor, DeltaTime * TransitionSpeed);
                    LightData.CurrentIntensity = FMath::FInterpTo(LightData.CurrentIntensity, LightData.TargetIntensity, DeltaTime, TransitionSpeed);
                    
                    PointLightComp->SetLightColor(LightData.CurrentColor);
                    PointLightComp->SetIntensity(LightData.CurrentIntensity);
                }
            }
        }
    }
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

void APuzzleManager::InitializePlayerHUD()
{
    if (PlayerHUDWidgetClass)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC)
        {
            PlayerHUDWidget = CreateWidget<UUserWidget>(PC, PlayerHUDWidgetClass);
            if (PlayerHUDWidget)
            {
                PlayerHUDWidget->AddToViewport();

                ObjectiveLabel = Cast<UTextBlock>(PlayerHUDWidget->GetWidgetFromName(TEXT("objective_label")));
                TaskLabel = Cast<UTextBlock>(PlayerHUDWidget->GetWidgetFromName(TEXT("task_label")));
            }
        }
    }
}

void APuzzleManager::RegisterMachines()
{
    for (int32 i = 0; i < PuzzleData.Num(); ++i)
    {
        if (PuzzleData[i].Machine && IsValid(PuzzleData[i].Machine))
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
        PuzzleData[PuzzleIndex].bIsActive = false;
        UpdatePointLightForPuzzle(PuzzleIndex, 100.0f);
        
        ActivateNextPuzzle();
    }

    if (ProgressBar && IsValid(ProgressBar))
    {
        ProgressBar->SetVisibility(ESlateVisibility::Hidden);
    }
    
    if (InteractLabel && IsValid(InteractLabel))
    {
        InteractLabel->SetVisibility(ESlateVisibility::Hidden);
    }

    CalculateGlobalProgression();

    bool bAllFixed = true;
    for (const FPuzzleData& Puzzle : PuzzleData)
    {
        if (Puzzle.Machine && IsValid(Puzzle.Machine) && !Puzzle.Machine->IsFixed())
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
            UpdatePointLightForPuzzle(PuzzleIndex, Progress * 100.0f);
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
    if (ObjectiveLabel && IsValid(ObjectiveLabel))
    {
        ObjectiveLabel->SetText(FText::FromString(TEXT("OBJECTIVE: ALL SYSTEMS OPERATIONAL")));
    }
    if (TaskLabel && IsValid(TaskLabel))
    {
        TaskLabel->SetText(FText::FromString(TEXT("All repairs completed successfully!")));
    }
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

FText APuzzleManager::GetCurrentObjective() const
{
    if (CurrentPuzzleIndex >= 0 && CurrentPuzzleIndex < PuzzleData.Num())
    {
        return PuzzleData[CurrentPuzzleIndex].ObjectiveText;
    }
    return FText::FromString(TEXT("OBJECTIVE: UNKNOWN"));
}

FText APuzzleManager::GetCurrentTask() const
{
    if (CurrentPuzzleIndex >= 0 && CurrentPuzzleIndex < PuzzleData.Num())
    {
        return PuzzleData[CurrentPuzzleIndex].TaskText;
    }
    return FText::FromString(TEXT("No active tasks"));
}

void APuzzleManager::ActivateNextPuzzle()
{
    for (int32 i = 0; i < PuzzleData.Num(); ++i)
    {
        if (!PuzzleData[i].bIsCompleted && !PuzzleData[i].bIsActive)
        {
            PuzzleData[i].bIsActive = true;
            CurrentPuzzleIndex = i;
            UpdatePlayerHUDLabels();
            return;
        }
    }
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

void APuzzleManager::UpdatePointLights()
{
    for (int32 i = 0; i < PointLights.Num(); ++i)
    {
        FPointLightData& LightData = PointLights[i];
        if (LightData.PointLight && LightData.AssociatedPuzzleIndex >= 0 && LightData.AssociatedPuzzleIndex < PuzzleData.Num())
        {
            float CompletionPercentage = PuzzleData[LightData.AssociatedPuzzleIndex].CompletionPercentage;
            UpdatePointLightForPuzzle(LightData.AssociatedPuzzleIndex, CompletionPercentage);
        }
    }
}

void APuzzleManager::UpdatePointLightForPuzzle(int32 PuzzleIndex, float CompletionPercentage)
{
    for (FPointLightData& LightData : PointLights)
    {
        if (LightData.AssociatedPuzzleIndex == PuzzleIndex && LightData.PointLight)
        {
            float Alpha = CompletionPercentage / 100.0f;
            Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

            FLinearColor NewTargetColor = FMath::Lerp(LightData.StartColor, LightData.EndColor, Alpha);
            float NewTargetIntensity = FMath::Lerp(LightData.StartIntensity, LightData.EndIntensity, Alpha);

            if (bUseInterpolation)
            {
                LightData.TargetColor = NewTargetColor;
                LightData.TargetIntensity = NewTargetIntensity;
            }
            else
            {
                UPointLightComponent* PointLightComp = Cast<UPointLightComponent>(LightData.PointLight->GetLightComponent());
                if (PointLightComp)
                {
                    PointLightComp->SetLightColor(NewTargetColor);
                    PointLightComp->SetIntensity(NewTargetIntensity);
                }
                LightData.CurrentColor = NewTargetColor;
                LightData.CurrentIntensity = NewTargetIntensity;
                LightData.TargetColor = NewTargetColor;
                LightData.TargetIntensity = NewTargetIntensity;
            }
        }
    }
}

void APuzzleManager::FindAndAssignPointLights()
{
    PointLights.Empty();
    
    TArray<AActor*> FoundPointLights;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APointLight::StaticClass(), FoundPointLights);
    
    for (int32 i = 0; i < FoundPointLights.Num(); ++i)
    {
        if (APointLight* PointLight = Cast<APointLight>(FoundPointLights[i]))
        {
            FPointLightData NewLightData;
            NewLightData.PointLight = PointLight;
            NewLightData.StartColor = FLinearColor::Red;
            NewLightData.EndColor = FLinearColor::Green;
            NewLightData.StartIntensity = 0.0f;
            NewLightData.EndIntensity = 3000.0f;
            NewLightData.AssociatedPuzzleIndex = i;
            
            NewLightData.CurrentColor = NewLightData.StartColor;
            NewLightData.CurrentIntensity = NewLightData.StartIntensity;
            NewLightData.TargetColor = NewLightData.StartColor;
            NewLightData.TargetIntensity = NewLightData.StartIntensity;
            
            PointLights.Add(NewLightData);
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

void APuzzleManager::UpdatePlayerHUDLabels()
{
    if (ObjectiveLabel && IsValid(ObjectiveLabel) && TaskLabel && IsValid(TaskLabel))
    {
        if (CurrentPuzzleIndex >= 0 && CurrentPuzzleIndex < PuzzleData.Num())
        {
            ObjectiveLabel->SetText(PuzzleData[CurrentPuzzleIndex].ObjectiveText);
            TaskLabel->SetText(PuzzleData[CurrentPuzzleIndex].TaskText);
        }
    }
}

void APuzzleManager::SortPuzzlesByOrder()
{
    PuzzleData.Sort([](const FPuzzleData& A, const FPuzzleData& B) {
        return A.PuzzleOrder < B.PuzzleOrder;
    });
}