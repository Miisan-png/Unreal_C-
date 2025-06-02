// PuzzleManager.h - Updated with smooth transitions
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PointLightComponent.h"
#include "Engine/PointLight.h"
#include "PuzzleManager.generated.h"

class AP_FixableMachine;
class UUserWidget;
class UProgressBar;
class UTextBlock;

USTRUCT(BlueprintType)
struct FPuzzleData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
    AP_FixableMachine* Machine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
    float CompletionPercentage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
    int32 PuzzleOrder = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
    bool bIsCompleted = false;
};

USTRUCT(BlueprintType)
struct FPointLightData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    APointLight* PointLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor StartColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor EndColor = FLinearColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float StartIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float EndIntensity = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    int32 AssociatedPuzzleIndex = -1;

    // Current interpolated values for smooth transitions
    UPROPERTY()
    FLinearColor CurrentColor;
    
    UPROPERTY()
    float CurrentIntensity;
    
    // Target values for interpolation
    UPROPERTY()
    FLinearColor TargetColor;
    
    UPROPERTY()
    float TargetIntensity;
};

UCLASS()
class FIRSTPERSONTEST_API APuzzleManager : public AActor
{
    GENERATED_BODY()

public:
    APuzzleManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> HUDWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
    TArray<FPuzzleData> PuzzleData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    TArray<FPointLightData> PointLights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
    float GlobalProgressionPercentage = 0.0f;

    // Transition settings for smooth color changes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float TransitionSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bUseInterpolation = true;

    UPROPERTY()
    UUserWidget* HUDWidget;

    UPROPERTY()
    UProgressBar* ProgressBar;

    UPROPERTY()
    UTextBlock* InteractLabel;

    void OnMachineStartFixing(AP_FixableMachine* Machine);
    void OnMachineStopFixing(AP_FixableMachine* Machine);
    void OnMachineFixed(AP_FixableMachine* Machine);
    void UpdateFixingProgress(float Progress);
    void ShowInteractionUI(bool bShow, const FText& Text = FText::GetEmpty());
    void HideProgressBar();

    UFUNCTION(BlueprintCallable, Category = "Puzzle")
    float GetPuzzleCompletionPercentage(int32 PuzzleIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Puzzle")
    float GetGlobalProgressionPercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdatePointLights();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void FindAndAssignPointLights();

protected:
    void InitializeHUD();
    void RegisterMachines();
    bool VerifyWidgets() const;
    void CalculateGlobalProgression();
    void UpdatePointLightForPuzzle(int32 PuzzleIndex, float CompletionPercentage);
    int32 FindPuzzleIndex(AP_FixableMachine* Machine);

    UFUNCTION()
    void OnAllMachinesFixed();

    UPROPERTY()
    AP_FixableMachine* CurrentHighlightedMachine;
};