#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oxygen System")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY()
	UUserWidget* HUDWidget;

	UPROPERTY()
	UImage* OxygenIcon;

	UPROPERTY()
	UTextBlock* OxygenLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oxygen System")
	FLinearColor NormalColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oxygen System")
	FLinearColor LowOxygenColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Oxygen System")
	void AddOxygen(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Oxygen System")
	void UpdateHUD();
};