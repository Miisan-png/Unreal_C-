#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "IInteractable.h"
#include "ArtifactActor.generated.h"

UCLASS()
class FIRSTPERSONTEST_API AArtifactActor : public AActor, public IInteractable
{
    GENERATED_BODY()
    
public:    
    AArtifactActor();
    virtual ~AArtifactActor();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
    FText ArtifactTitle;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
    FText ArtifactText;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Artifact")
    TSubclassOf<UUserWidget> ArtifactWidgetClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    UMaterialInterface* HighlightMaterial;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FText InteractionText = FText::FromString(TEXT("[E] Read Artifact"));
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FText CloseInteractionText = FText::FromString(TEXT("[E] Close"));
    
    UPROPERTY()
    UUserWidget* ArtifactWidget;
    
    UPROPERTY()
    UTextBlock* TitleLabel;
    
    UPROPERTY()
    UTextBlock* ContentLabel;
    
    UPROPERTY()
    UTextBlock* InteractionLabel;
    
    UPROPERTY()
    class UImage* Image_1;
    
    UPROPERTY()
    class UImage* Image_0;
    
    UPROPERTY()
    class UImage* Image;
    
    UPROPERTY()
    UMaterialInterface* OriginalMaterial;
    
    UPROPERTY()
    bool bCanInteract = false;
    
    UPROPERTY()
    bool bIsWidgetOpen = false;
    
    UPROPERTY()
    bool bShowingInteractionOnly = false;
    
    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    
    UFUNCTION()
    void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
    
    UFUNCTION(BlueprintCallable, Category = "Artifact")
    void ShowInteractionLabel();
    
    UFUNCTION(BlueprintCallable, Category = "Artifact")
    void ShowFullArtifactWidget();
    
    UFUNCTION(BlueprintCallable, Category = "Artifact")
    void HideArtifactWidget();
    
    virtual void OnHighlight_Implementation() override;
    virtual void OnUnhighlight_Implementation() override;
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual bool CanInteract_Implementation() const override;
    virtual FText GetInteractionText_Implementation() const override;
};