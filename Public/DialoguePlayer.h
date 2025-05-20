// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Engine/LevelScriptActor.h"
#include "Kismet/GameplayStatics.h"
#include "DialoguePlayer.generated.h"

// Struct to hold individual dialogue entry data
USTRUCT(BlueprintType)
struct FDialogueEntry
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString Text;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FLinearColor TextColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float TypeSpeed = 0.05f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DelayAfter = 1.0f;
};

UCLASS()
class FIRSTPERSONTEST_API ADialoguePlayer : public AActor
{
    GENERATED_BODY()
    
public:    
    ADialoguePlayer();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    FString CurrentText;
    FString TargetText;
    float TypewriterDelay;
    float CurrentDelay;
    int32 CurrentCharIndex;
    bool bIsTyping;
    int32 CurrentDialogueIndex;
    FLinearColor CurrentTextColor;
    bool bAllowInput;
    
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSubclassOf<UUserWidget> DialogueWidgetClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FDialogueEntry> DialogueEntries;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float IntroDelay = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FName NextLevelName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bContinueOnInput = true;
    
    UPROPERTY()
    UUserWidget* DialogueWidget;
    
    UPROPERTY()
    UTextBlock* TextLabel;
    
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartDialogue();
    
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AdvanceDialogue();
    
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SkipTypewriter();
    
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetDialogueText(const FString& NewText, const FLinearColor& TextColor, float Speed);
    
    UFUNCTION()
    void OnTypewriterComplete();
    
    UFUNCTION()
    void ShowNextDialogue();
    
    UFUNCTION()
    void ChangeScene();
    
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void HandleInput();
    
    // Setup input bindings
    void SetupInputBindings();
};