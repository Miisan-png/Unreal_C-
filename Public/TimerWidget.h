// TimerWidget.h
// A timer widget that changes color as time runs out

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Styling/SlateColor.h"
#include "TimerWidget.generated.h"

/**
 * 
 */
UCLASS()
class FIRSTPERSONTEST_API UTimerWidget : public UUserWidget
{
    GENERATED_BODY()
    
protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    
public:
    // The text block that displays the timer
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TimerText;
    
    // Initial time in seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer", meta = (ClampMin = "0.0"))
    float InitialTime = 60.0f;
    
    // Current time remaining
    UPROPERTY(BlueprintReadOnly, Category = "Timer")
    float CurrentTime;
    
    // Whether the timer is active
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
    bool bIsActive = true;
    
    // Whether the timer should loop
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
    bool bLoop = false;
    
    // The color when timer is at max time
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor NormalColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    // The color when timer is about to run out
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor WarningColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
    
    // The percentage threshold when the color starts to change
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WarningThreshold = 0.25f;
    
    // The format string for displaying the time (mm:ss)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString TimeFormat = TEXT("{0}:{1}");
    
    // Whether to show milliseconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bShowMilliseconds = false;
    
    // Start the timer
    UFUNCTION(BlueprintCallable, Category = "Timer")
    void StartTimer();
    
    // Pause the timer
    UFUNCTION(BlueprintCallable, Category = "Timer")
    void PauseTimer();
    
    // Reset the timer to initial time
    UFUNCTION(BlueprintCallable, Category = "Timer")
    void ResetTimer();
    
    // Set a new duration
    UFUNCTION(BlueprintCallable, Category = "Timer")
    void SetDuration(float NewDuration);
    
    // Called when timer completes
    UFUNCTION(BlueprintNativeEvent, Category = "Timer")
    void OnTimerComplete();
    
    // Function to update the timer text display
    void UpdateTimerDisplay();
};