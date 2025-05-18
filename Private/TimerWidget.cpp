// TimerWidget.cpp
// Implementation of the timer widget that changes color as time runs out

#include "TimerWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetTextLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Styling/SlateColor.h"

void UTimerWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Initialize timer
    CurrentTime = InitialTime;
    
    // Initialize timer display
    UpdateTimerDisplay();
}

void UTimerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    
    if (bIsActive)
    {
        // Decrease time
        CurrentTime = FMath::Max(CurrentTime - InDeltaTime, 0.0f);
        
        // Update display
        UpdateTimerDisplay();
        
        // Check if timer completed
        if (CurrentTime <= 0.0f)
        {
            bIsActive = false;
            OnTimerComplete();
            
            // Loop if needed
            if (bLoop)
            {
                ResetTimer();
                StartTimer();
            }
        }
    }
}

void UTimerWidget::StartTimer()
{
    bIsActive = true;
}

void UTimerWidget::PauseTimer()
{
    bIsActive = false;
}

void UTimerWidget::ResetTimer()
{
    CurrentTime = InitialTime;
    UpdateTimerDisplay();
}

void UTimerWidget::SetDuration(float NewDuration)
{
    InitialTime = FMath::Max(NewDuration, 0.0f);
    ResetTimer();
}

void UTimerWidget::OnTimerComplete_Implementation()
{
    // Can be overridden in Blueprint
}

void UTimerWidget::UpdateTimerDisplay()
{
    if (!TimerText)
    {
        return;
    }
    
    // Calculate minutes and seconds
    int32 TotalSeconds = FMath::FloorToInt(CurrentTime);
    int32 Minutes = TotalSeconds / 60;
    int32 Seconds = TotalSeconds % 60;
    
    FString TimeString;
    
    if (bShowMilliseconds)
    {
        // Include milliseconds
        int32 Milliseconds = FMath::FloorToInt((CurrentTime - TotalSeconds) * 100);
        TimeString = FString::Printf(TEXT("%02d:%02d.%02d"), Minutes, Seconds, Milliseconds);
    }
    else
    {
        // Format as mm:ss
        TimeString = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
    }
    
    // Set the text
    TimerText->SetText(FText::FromString(TimeString));
    
    // Calculate the color based on remaining time percentage
    float TimePercentage = CurrentTime / InitialTime;
    
    if (TimePercentage <= WarningThreshold)
    {
        // Interpolate color from warning to normal based on time remaining
        float Alpha = TimePercentage / WarningThreshold;
        FLinearColor CurrentColor = FLinearColor::LerpUsingHSV(WarningColor, NormalColor, Alpha);
        
        // Add pulsating effect when close to zero
        if (TimePercentage < WarningThreshold / 2.0f)
        {
            // Pulse frequency increases as time gets lower
            float PulseRate = 2.0f + (1.0f - TimePercentage) * 8.0f;
            float PulseAmount = 0.2f * (1.0f - TimePercentage) * FMath::Sin(GetWorld()->GetTimeSeconds() * PulseRate);
            
            // Make color pulse brighter
            CurrentColor.R = FMath::Min(CurrentColor.R + PulseAmount, 1.0f);
            CurrentColor.G = FMath::Min(CurrentColor.G + PulseAmount, 1.0f);
            CurrentColor.B = FMath::Min(CurrentColor.B + PulseAmount, 1.0f);
        }
        
        TimerText->SetColorAndOpacity(CurrentColor);
    }
    else
    {
        // Use normal color
        TimerText->SetColorAndOpacity(NormalColor);
    }
}