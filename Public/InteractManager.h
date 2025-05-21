#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IInteractable.h"
#include "InteractManager.generated.h"

UCLASS()
class FIRSTPERSONTEST_API AInteractManager : public AActor
{
	GENERATED_BODY()
    
public:    
	AInteractManager();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void RegisterPlayerController(APlayerController* PC);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void RegisterPlayerCharacter(ACharacter* Character);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void RegisterInteractPromptWidget(class UUserWidget* PromptWidget, class UTextBlock* PromptText);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool IsLookingAtInteractable() const;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void TryInteract();
    
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void StartInteract();
    
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void StopInteract();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionRange = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bShowDebugRaycast = false;

	UPROPERTY()
	APlayerController* PlayerController;

	UPROPERTY()
	ACharacter* PlayerCharacter;

	UPROPERTY()
	UUserWidget* PromptWidget;

	UPROPERTY()
	UTextBlock* PromptText;

	UPROPERTY()
	TScriptInterface<IInteractable> CurrentInteractable;
    
	UPROPERTY()
	bool bIsInteracting = false;

	void PerformInteractionRaycast();
	void UpdatePromptVisibility(bool bVisible, const FText& Text = FText::GetEmpty());
};