#include "PuzzleManager.h"
#include "P_FixableMachine.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

APuzzleManager::APuzzleManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void APuzzleManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeHUD();
    RegisterMachines();
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

                // Get the progress bar and interaction label widgets
                ProgressBar = Cast<UProgressBar>(HUDWidget->GetWidgetFromName(TEXT("ProgressBar")));
                InteractLabel = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("InteractLabel")));

                // Hide widgets initially
                if (ProgressBar)
                {
                    ProgressBar->SetVisibility(ESlateVisibility::Hidden);
                    UE_LOG(LogTemp, Warning, TEXT("ProgressBar found and initialized"));
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("ProgressBar widget not found! Make sure your HUD has a ProgressBar named 'ProgressBar'"));
                }

                if (InteractLabel)
                {
                    InteractLabel->SetVisibility(ESlateVisibility::Hidden);
                    UE_LOG(LogTemp, Warning, TEXT("InteractLabel found and initialized"));
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("InteractLabel widget not found! Make sure your HUD has a TextBlock named 'InteractLabel'"));
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to create HUD Widget!"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to get PlayerController!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("HUDWidgetClass not set in PuzzleManager!"));
    }
}

void APuzzleManager::RegisterMachines()
{
    // Find all fixable machines in the level if not set
    if (FixableMachines.Num() == 0)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AP_FixableMachine::StaticClass(), FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            if (AP_FixableMachine* Machine = Cast<AP_FixableMachine>(Actor))
            {
                FixableMachines.Add(Machine);
                Machine->SetPuzzleManager(this);
            }
        }
    }
    else
    {
        // Set this manager as the puzzle manager for all machines
        for (AP_FixableMachine* Machine : FixableMachines)
        {
            if (Machine)
            {
                Machine->SetPuzzleManager(this);
            }
        }
    }
}

void APuzzleManager::OnMachineStartFixing(AP_FixableMachine* Machine)
{
    if (!VerifyWidgets()) return;

    UE_LOG(LogTemp, Warning, TEXT("Machine started fixing"));
    if (ProgressBar)
    {
        ProgressBar->SetVisibility(ESlateVisibility::Visible);
        ProgressBar->SetPercent(0.0f);
        UE_LOG(LogTemp, Warning, TEXT("Progress bar shown and set to 0"));
    }
}

void APuzzleManager::OnMachineStopFixing(AP_FixableMachine* Machine)
{
    if (!VerifyWidgets()) return;

    UE_LOG(LogTemp, Warning, TEXT("Machine stopped fixing"));
    if (ProgressBar)
    {
        ProgressBar->SetVisibility(ESlateVisibility::Hidden);
        ProgressBar->SetPercent(0.0f);
        UE_LOG(LogTemp, Warning, TEXT("Progress bar hidden"));
    }
}

void APuzzleManager::OnMachineFixed(AP_FixableMachine* Machine)
{
    if (!VerifyWidgets()) return;

    UE_LOG(LogTemp, Warning, TEXT("Machine fixed"));
    if (ProgressBar)
    {
        ProgressBar->SetVisibility(ESlateVisibility::Hidden);
    }

    // Check if all machines are fixed
    bool bAllFixed = true;
    for (AP_FixableMachine* FixableMachine : FixableMachines)
    {
        if (FixableMachine && !FixableMachine->IsFixed())
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
        UE_LOG(LogTemp, Display, TEXT("Progress updated: %f"), Progress);
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
            UE_LOG(LogTemp, Display, TEXT("Showing interaction text: %s"), *Text.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Display, TEXT("Hiding interaction text"));
        }
    }
}

bool APuzzleManager::VerifyWidgets() const
{
    if (!HUDWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("HUDWidget is null!"));
        return false;
    }
    if (!ProgressBar)
    {
        UE_LOG(LogTemp, Error, TEXT("ProgressBar is null!"));
        return false;
    }
    if (!InteractLabel)
    {
        UE_LOG(LogTemp, Error, TEXT("InteractLabel is null!"));
        return false;
    }
    return true;
}

void APuzzleManager::OnAllMachinesFixed()
{
    // You can implement what happens when all machines are fixed
    // For example, trigger an event, open a door, etc.
}