#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PuzzleManager.generated.h"

class AP_FixableMachine;
class UUserWidget;
class UProgressBar;
class UTextBlock;

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
	TArray<AP_FixableMachine*> FixableMachines;

	UPROPERTY()
	UUserWidget* HUDWidget;

	UPROPERTY()
	UProgressBar* ProgressBar;

	UPROPERTY()
	UTextBlock* InteractLabel;

	// Called when a machine starts being fixed
	void OnMachineStartFixing(AP_FixableMachine* Machine);

	// Called when a machine stops being fixed
	void OnMachineStopFixing(AP_FixableMachine* Machine);

	// Called when a machine is completely fixed
	void OnMachineFixed(AP_FixableMachine* Machine);

	// Updates the progress bar
	void UpdateFixingProgress(float Progress);

	// Show/hide interaction UI
	void ShowInteractionUI(bool bShow, const FText& Text = FText::GetEmpty());

protected:
	void InitializeHUD();
	void RegisterMachines();
	bool VerifyWidgets() const;

	UFUNCTION()
	void OnAllMachinesFixed();

	// Keep track of currently highlighted machine
	UPROPERTY()
	AP_FixableMachine* CurrentHighlightedMachine;
};