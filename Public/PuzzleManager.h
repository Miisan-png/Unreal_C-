#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SpotLightComponent.h"
#include "Engine/SpotLight.h"
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
struct FSpotLightData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    ASpotLight* SpotLight;

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
};

UCLASS()
class FIRSTPERSONTEST_API APuzzleManager : public AActor
{
    GENERATED_BODY()

public:
    APuzzleManager();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> HUDWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
    TArray<FPuzzleData> PuzzleData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    TArray<FSpotLightData> SpotLights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
    float GlobalProgressionPercentage = 0.0f;

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
    void UpdateSpotLights();

protected:
    void InitializeHUD();
    void RegisterMachines();
    bool VerifyWidgets() const;
    void CalculateGlobalProgression();
    void UpdateSpotLightForPuzzle(int32 PuzzleIndex, float CompletionPercentage);
    int32 FindPuzzleIndex(AP_FixableMachine* Machine);

    UFUNCTION()
    void OnAllMachinesFixed();

    UPROPERTY()
    AP_FixableMachine* CurrentHighlightedMachine;
};