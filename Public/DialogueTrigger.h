#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/Character.h"
#include "DialogueTrigger.generated.h"

UENUM(BlueprintType)
enum class EDialogueState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    FadingIn    UMETA(DisplayName = "Fading In"),
    Typing      UMETA(DisplayName = "Typing"),
    Displaying  UMETA(DisplayName = "Displaying"),
    FadingOut   UMETA(DisplayName = "Fading Out")
};

UCLASS()
class FIRSTPERSONTEST_API ADialogueTrigger : public AActor
{
    GENERATED_BODY()
    
public:    
    ADialogueTrigger();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* TriggerBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText = TEXT("Hello, welcome to the station!");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float TypewriterSpeed = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float FadeInDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float FadeOutDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bCanReplay = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSubclassOf<UUserWidget> DialogueWidgetClass;

    UPROPERTY()
    UUserWidget* DialogueWidget;

    UPROPERTY()
    UTextBlock* DialogueLabel;

    UPROPERTY()
    bool bHasBeenTriggered = false;

    UPROPERTY()
    bool bIsPlaying = false;

    UPROPERTY()
    FString CurrentText;

    UPROPERTY()
    int32 CurrentCharIndex;

    UPROPERTY()
    float TypewriterTimer;

    UPROPERTY()
    float FadeTimer;

    UPROPERTY()
    float DisplayTimer;

    UPROPERTY()
    EDialogueState CurrentState = EDialogueState::Idle;

    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ResetDialogue();

private:
    void UpdateDialogue(float DeltaTime);
    void SetDialogueWidgetOpacity(float Opacity);
};