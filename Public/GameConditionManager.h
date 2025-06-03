#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/Character.h"
#include "IInteractable.h"
#include "PuzzleManager.h"
#include "GameConditionManager.generated.h"

UCLASS()
class FIRSTPERSONTEST_API AGameConditionManager : public AActor, public IInteractable
{
    GENERATED_BODY()
    
public:    
    AGameConditionManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* TriggerBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Condition")
    FName NextLevelName = TEXT("NextLevel");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Condition")
    bool bRequireAllPuzzlesSolved = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Condition")
    APuzzleManager* PuzzleManagerRef;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> InteractionWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    FText InteractionText = FText::FromString(TEXT("[E] Continue to Next Area"));

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    FText PuzzlesIncompleteText = FText::FromString(TEXT("Complete all repairs first"));

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    UMaterialInterface* AvailableMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    UMaterialInterface* UnavailableMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    UMaterialInterface* HighlightMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* ActivationSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* DeniedSound;

    UPROPERTY()
    UUserWidget* InteractionWidget;

    UPROPERTY()
    UTextBlock* InteractionLabel;

    UPROPERTY()
    UMaterialInterface* OriginalMaterial;

    UPROPERTY()
    bool bCanInteract = false;

    UPROPERTY()
    bool bPlayerInRange = false;

    UPROPERTY()
    bool bAllPuzzlesSolved = false;

    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION(BlueprintCallable, Category = "Game Condition")
    bool CheckAllConditionsMet();

    UFUNCTION(BlueprintCallable, Category = "Game Condition")
    bool CheckPuzzleConditions();

    UFUNCTION(BlueprintCallable, Category = "Game Condition")
    void UpdateInteractionAvailability();

    UFUNCTION(BlueprintCallable, Category = "Game Condition")
    void ChangeToNextLevel();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowInteractionPrompt(bool bShow);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void UpdateInteractionText();

    virtual void OnHighlight_Implementation() override;
    virtual void OnUnhighlight_Implementation() override;
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual bool CanInteract_Implementation() const override;
    virtual FText GetInteractionText_Implementation() const override;

    UFUNCTION(BlueprintImplementableEvent, Category = "Game Condition")
    void OnConditionsMetChanged(bool bConditionsMet);

    UFUNCTION(BlueprintImplementableEvent, Category = "Game Condition")
    void OnLevelTransition();

protected:
    void FindPuzzleManager();
    void UpdateVisualFeedback();
};