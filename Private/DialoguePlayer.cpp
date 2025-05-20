// Fill out your copyright notice in the Description page of Project Settings.

#include "DialoguePlayer.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

ADialoguePlayer::ADialoguePlayer()
{
    PrimaryActorTick.bCanEverTick = true;
    
    CurrentCharIndex = 0;
    bIsTyping = false;
    CurrentDelay = 0.0f;
    CurrentDialogueIndex = -1; // Start at -1 so first increment goes to 0
    CurrentTextColor = FLinearColor::White;
    bAllowInput = false;
}

void ADialoguePlayer::BeginPlay()
{
    Super::BeginPlay();
    
    if (DialogueWidgetClass)
    {
        DialogueWidget = CreateWidget<UUserWidget>(GetWorld(), DialogueWidgetClass);
        if (DialogueWidget)
        {
            DialogueWidget->AddToViewport();
            
            // Try to find the TextBlock named "text_label" in the widget
            TextLabel = Cast<UTextBlock>(DialogueWidget->GetWidgetFromName(FName("text_label")));
            
            if (TextLabel)
            {
                // Set initial empty text
                TextLabel->SetText(FText::FromString(""));
                
                // Setup input bindings if needed
                if (bContinueOnInput)
                {
                    SetupInputBindings();
                }
                
                // Start dialogue with intro delay
                FTimerHandle TimerHandle;
                GetWorldTimerManager().SetTimer(TimerHandle, this, &ADialoguePlayer::StartDialogue, IntroDelay, false);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("TextLabel not found in DialogueWidget!"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create DialogueWidget!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("DialogueWidgetClass not set!"));
    }
}

void ADialoguePlayer::SetupInputBindings()
{
    // Get the player controller
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PlayerController)
    {
        // Enable input for this actor
        EnableInput(PlayerController);
        
        // Make sure we have a valid input component
        if (InputComponent)
        {
            // Bind the "Action" input to our HandleInput function
            InputComponent->BindAction("Action", IE_Pressed, this, &ADialoguePlayer::HandleInput);
            UE_LOG(LogTemp, Display, TEXT("Input binding setup complete"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to set up input binding: InputComponent is null"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to set up input binding: PlayerController is null"));
    }
}

void ADialoguePlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsTyping && TextLabel)
    {
        CurrentDelay -= DeltaTime;
        
        if (CurrentDelay <= 0.0f)
        {
            if (CurrentCharIndex < TargetText.Len())
            {
                // Add next character
                CurrentText += TargetText[CurrentCharIndex];
                TextLabel->SetText(FText::FromString(CurrentText));
                CurrentCharIndex++;
                
                // Reset delay for next character
                CurrentDelay = TypewriterDelay;
            }
            else
            {
                // Typewriter effect complete
                bIsTyping = false;
                bAllowInput = true;
                OnTypewriterComplete();
            }
        }
    }
}

void ADialoguePlayer::StartDialogue()
{
    if (TextLabel && DialogueEntries.Num() > 0)
    {
        UE_LOG(LogTemp, Display, TEXT("Starting dialogue sequence with %d entries"), DialogueEntries.Num());
        
        // Reset indices and advance to first dialogue
        CurrentDialogueIndex = -1;
        AdvanceDialogue();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot start dialogue: TextLabel is %s, DialogueEntries count: %d"), 
               TextLabel ? TEXT("valid") : TEXT("null"), DialogueEntries.Num());
    }
}

void ADialoguePlayer::HandleInput()
{
    UE_LOG(LogTemp, Display, TEXT("HandleInput called. bIsTyping=%s, bAllowInput=%s"), 
           bIsTyping ? TEXT("true") : TEXT("false"),
           bAllowInput ? TEXT("true") : TEXT("false"));
    
    if (!bAllowInput)
    {
        return;
    }
    
    if (bIsTyping)
    {
        // Skip the current typewriter animation
        SkipTypewriter();
    }
    else
    {
        // Go to next dialogue
        AdvanceDialogue();
    }
}

void ADialoguePlayer::AdvanceDialogue()
{
    // Move to the next dialogue entry
    CurrentDialogueIndex++;
    
    UE_LOG(LogTemp, Display, TEXT("Advancing to dialogue index %d of %d total entries"), 
           CurrentDialogueIndex, DialogueEntries.Num());
    
    // Check if we have more dialogue entries
    if (CurrentDialogueIndex < DialogueEntries.Num())
    {
        // Show next dialogue
        const FDialogueEntry& Entry = DialogueEntries[CurrentDialogueIndex];
        UE_LOG(LogTemp, Display, TEXT("Showing dialogue: \"%s\""), *Entry.Text);
        SetDialogueText(Entry.Text, Entry.TextColor, Entry.TypeSpeed);
    }
    else
    {
        // No more dialogues, change scene
        UE_LOG(LogTemp, Display, TEXT("No more dialogues, changing scene"));
        ChangeScene();
    }
}

void ADialoguePlayer::SetDialogueText(const FString& NewText, const FLinearColor& TextColor, float Speed)
{
    if (TextLabel)
    {
        // Initialize typewriter variables
        TargetText = NewText;
        CurrentText = "";
        CurrentCharIndex = 0;
        CurrentDelay = 0.0f;
        TypewriterDelay = Speed;
        bIsTyping = true;
        bAllowInput = false;
        CurrentTextColor = TextColor;
        
        // Apply text color
        TextLabel->SetColorAndOpacity(TextColor);
        
        // Clear existing text
        TextLabel->SetText(FText::FromString(""));
        
        UE_LOG(LogTemp, Display, TEXT("Set dialogue text: \"%s\" with speed %f"), *NewText, Speed);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot set dialogue text: TextLabel is null"));
    }
}

void ADialoguePlayer::SkipTypewriter()
{
    if (bIsTyping && TextLabel)
    {
        UE_LOG(LogTemp, Display, TEXT("Skipping typewriter animation"));
        
        // Skip to the end of the text
        CurrentText = TargetText;
        TextLabel->SetText(FText::FromString(CurrentText));
        bIsTyping = false;
        bAllowInput = true;
        
        OnTypewriterComplete();
    }
}

void ADialoguePlayer::OnTypewriterComplete()
{
    UE_LOG(LogTemp, Display, TEXT("Typewriter animation complete for dialogue %d"), CurrentDialogueIndex);
    
    // If automatic progression is enabled (not waiting for input)
    if (!bContinueOnInput)
    {
        // Get the current dialogue entry
        if (CurrentDialogueIndex < DialogueEntries.Num())
        {
            const FDialogueEntry& CurrentEntry = DialogueEntries[CurrentDialogueIndex];
            float DelayToNext = CurrentEntry.DelayAfter;
            
            UE_LOG(LogTemp, Display, TEXT("Auto-advancing to next dialogue after %f seconds"), DelayToNext);
            
            // Set timer to show next dialogue after delay
            FTimerHandle TimerHandle;
            GetWorldTimerManager().SetTimer(TimerHandle, this, &ADialoguePlayer::ShowNextDialogue, DelayToNext, false);
        }
    }
    // If waiting for input, do nothing and wait for HandleInput to be called
    else
    {
        UE_LOG(LogTemp, Display, TEXT("Waiting for user input to advance dialogue"));
    }
}

void ADialoguePlayer::ShowNextDialogue()
{
    UE_LOG(LogTemp, Display, TEXT("ShowNextDialogue called"));
    AdvanceDialogue();
}

void ADialoguePlayer::ChangeScene()
{
    UE_LOG(LogTemp, Display, TEXT("ChangeScene called, transitioning to %s"), *NextLevelName.ToString());
    
    // First hide the dialogue widget if it exists
    if (DialogueWidget)
    {
        DialogueWidget->RemoveFromViewport();
    }
    
    // Then change to the next level if specified
    if (!NextLevelName.IsNone())
    {
        UGameplayStatics::OpenLevel(this, NextLevelName);
    }
}