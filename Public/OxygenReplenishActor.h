// OxygenReplenishActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PlayerOxygenSystem.h"
#include "IInteractable.h"
#include "OxygenReplenishActor.generated.h"

UCLASS()
class FIRSTPERSONTEST_API AOxygenReplenishActor : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	AOxygenReplenishActor();

protected:
	virtual void BeginPlay() override;

public:	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* InteractionSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oxygen System")
	float OxygenAmount = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oxygen System")
	bool bDestroyAfterUse = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> InteractWidgetClass;
	
	UPROPERTY()
	UUserWidget* InteractWidget;
	
	UPROPERTY()
	UTextBlock* ReplenishLabel;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    UMaterialInterface* HighlightMaterial;
    
    UPROPERTY()
    UMaterialInterface* OriginalMaterial;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FText InteractionText = FText::FromString(TEXT("[E] REPLENISH OXYGEN"));

	UFUNCTION()
	void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable, Category = "Oxygen System")
	void PerformInteraction(AActor* Interactor);
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowInteractPrompt(bool bShow);
	
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool IsInteractable() const { return bCanInteract; }
    
    // IInteractable interface
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void OnHighlight();
    virtual void OnHighlight_Implementation() override;
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void OnUnhighlight();
    virtual void OnUnhighlight_Implementation() override;
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void Interact(AActor* Interactor);
    virtual void Interact_Implementation(AActor* Interactor) override;
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    bool CanInteract() const;
    virtual bool CanInteract_Implementation() const override;
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    FText GetInteractionText() const;
    virtual FText GetInteractionText_Implementation() const override;
    // End of IInteractable interface

private:
	bool bCanInteract;
	APlayerOxygenSystem* PlayerOxygenSystem;
};