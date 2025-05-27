#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"
#include "PlayerOxygenSystem.generated.h"

UCLASS()
class FIRSTPERSONTEST_API APlayerOxygenSystem : public AActor
{
	GENERATED_BODY()
	
public:	
	APlayerOxygenSystem();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oxygen System")
	float MaxOxygen = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oxygen System")
	float CurrentOxygen = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oxygen System")
	float OxygenDecreaseRate = 1.0f;

	// Screen tint threshold (when red tint starts)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oxygen System", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float LowOxygenThreshold = 30.0f;

	// Delay before restarting the game when oxygen reaches 0
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oxygen System", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float GameOverDelay = 2.0f;

	// Level to restart to (leave empty for current level)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oxygen System")
	FName RestartLevelName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oxygen System")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	// Screen overlay widget class for red tint effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oxygen System")
	TSubclassOf<UUserWidget> ScreenOverlayWidgetClass;

	UPROPERTY()
	UUserWidget* HUDWidget;

	UPROPERTY()
	UUserWidget* ScreenOverlayWidget;

	UPROPERTY()
	UImage* OxygenIcon;

	UPROPERTY()
	UTextBlock* OxygenLabel;

	UPROPERTY()
	UImage* ScreenTintImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oxygen System")
	FLinearColor NormalColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oxygen System")
	FLinearColor LowOxygenColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oxygen System")
	FLinearColor ScreenTintColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.5f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oxygen System", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxScreenTintAlpha = 0.7f;

	UFUNCTION(BlueprintCallable, Category = "Oxygen System")
	void AddOxygen(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Oxygen System")
	void UpdateHUD();

	UFUNCTION(BlueprintCallable, Category = "Oxygen System")
	void UpdateScreenTint();

	UFUNCTION(BlueprintCallable, Category = "Oxygen System")
	void RestartGame();
	UFUNCTION(BlueprintPure, Category = "Oxygen System")
	float GetOxygenPercentage() const { return CurrentOxygen / MaxOxygen; }
	UFUNCTION(BlueprintPure, Category = "Oxygen System")
	bool IsOxygenCritical() const { return CurrentOxygen <= LowOxygenThreshold; }

private:
	FTimerHandle GameOverTimerHandle;
	bool bGameOverTriggered = false;

	void TriggerGameOver();
};