// OxygenReplenishActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "PlayerOxygenSystem.h"
#include "OxygenReplenishActor.generated.h"

UCLASS()
class FIRSTPERSONTEST_API AOxygenReplenishActor : public AActor
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

	UFUNCTION()
	void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable, Category = "Oxygen System")
	void Interact(AActor* Interactor);
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowInteractPrompt(bool bShow);
	
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool IsInteractable() const { return bCanInteract; }

private:
	bool bCanInteract;
	APlayerOxygenSystem* PlayerOxygenSystem;
};