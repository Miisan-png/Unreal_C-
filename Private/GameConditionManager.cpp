#include "GameConditionManager.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Sound/SoundBase.h"

AGameConditionManager::AGameConditionManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create trigger box
    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    RootComponent = TriggerBox;
    TriggerBox->SetBoxExtent(FVector(150.0f, 150.0f, 100.0f));
    TriggerBox->SetCollisionProfileName(TEXT("Trigger"));

    // Create mesh component
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    
    // Use a default cube mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshFinder.Succeeded())
    {
        MeshComponent->SetStaticMesh(CubeMeshFinder.Object);
        MeshComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, 2.0f));
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // Initialize variables
    InteractionWidget = nullptr;
    InteractionLabel = nullptr;
    OriginalMaterial = nullptr;
    PuzzleManagerRef = nullptr;
    bCanInteract = false;
    bPlayerInRange = false;
    bAllPuzzlesSolved = false;
}

void AGameConditionManager::BeginPlay()
{
    Super::BeginPlay();

    // Bind trigger events
    if (TriggerBox)
    {
        TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AGameConditionManager::OnTriggerBeginOverlap);
        TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AGameConditionManager::OnTriggerEndOverlap);
    }

    // Store original material
    if (MeshComponent && MeshComponent->GetMaterial(0))
    {
        OriginalMaterial = MeshComponent->GetMaterial(0);
    }

    // Find puzzle manager if not assigned
    if (!PuzzleManagerRef)
    {
        FindPuzzleManager();
    }

    // Create interaction widget
    if (InteractionWidgetClass)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC)
        {
            InteractionWidget = CreateWidget<UUserWidget>(PC, InteractionWidgetClass);
            if (InteractionWidget)
            {
                InteractionWidget->AddToViewport(100);
                InteractionWidget->SetVisibility(ESlateVisibility::Hidden);
                
                // Find the text label in the widget
                InteractionLabel = Cast<UTextBlock>(InteractionWidget->GetWidgetFromName(TEXT("interaction_label")));
                if (!InteractionLabel)
                {
                    // Try alternative names
                    InteractionLabel = Cast<UTextBlock>(InteractionWidget->GetWidgetFromName(TEXT("InteractionLabel")));
                }
            }
        }
    }

    // Initial condition check
    UpdateInteractionAvailability();
}

void AGameConditionManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Continuously check puzzle conditions
    bool bPreviousState = bAllPuzzlesSolved;
    UpdateInteractionAvailability();
    
    // If conditions changed, trigger blueprint event
    if (bPreviousState != bAllPuzzlesSolved)
    {
        OnConditionsMetChanged(CheckAllConditionsMet());
    }
}

void AGameConditionManager::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
    {
        bPlayerInRange = true;
        UpdateInteractionAvailability();
        
        if (bCanInteract)
        {
            ShowInteractionPrompt(true);
        }
    }
}

void AGameConditionManager::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
    {
        bPlayerInRange = false;
        bCanInteract = false;
        ShowInteractionPrompt(false);
    }
}

bool AGameConditionManager::CheckAllConditionsMet()
{
    // Check puzzle conditions
    if (bRequireAllPuzzlesSolved && !CheckPuzzleConditions())
    {
        return false;
    }

    // Player must be in range
    if (!bPlayerInRange)
    {
        return false;
    }

    // Add any additional conditions here in the future
    return true;
}

bool AGameConditionManager::CheckPuzzleConditions()
{
    if (!PuzzleManagerRef)
    {
        return !bRequireAllPuzzlesSolved; // If no puzzle manager, return true only if puzzles aren't required
    }

    // Check if all puzzles are completed
    for (const FPuzzleData& Puzzle : PuzzleManagerRef->PuzzleData)
    {
        if (!Puzzle.bIsCompleted)
        {
            return false;
        }
    }

    return true;
}

void AGameConditionManager::UpdateInteractionAvailability()
{
    bAllPuzzlesSolved = CheckPuzzleConditions();
    bCanInteract = CheckAllConditionsMet();
    
    UpdateVisualFeedback();
    UpdateInteractionText();
}

void AGameConditionManager::ChangeToNextLevel()
{
    UE_LOG(LogTemp, Warning, TEXT("GameConditionManager: Changing to level %s"), *NextLevelName.ToString());
    
    // Trigger blueprint event before transition
    OnLevelTransition();
    
    // Hide UI
    ShowInteractionPrompt(false);
    
    // Play activation sound
    if (ActivationSound)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), ActivationSound);
    }
    
    // Change level
    if (!NextLevelName.IsNone())
    {
        UGameplayStatics::OpenLevel(this, NextLevelName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameConditionManager: NextLevelName is not set!"));
    }
}

void AGameConditionManager::ShowInteractionPrompt(bool bShow)
{
    if (InteractionWidget)
    {
        InteractionWidget->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }
}

void AGameConditionManager::UpdateInteractionText()
{
    if (InteractionLabel)
    {
        if (bAllPuzzlesSolved)
        {
            InteractionLabel->SetText(InteractionText);
        }
        else
        {
            InteractionLabel->SetText(PuzzlesIncompleteText);
        }
    }
}

void AGameConditionManager::OnHighlight_Implementation()
{
    if (HighlightMaterial && MeshComponent)
    {
        MeshComponent->SetMaterial(0, HighlightMaterial);
    }
    
    if (bPlayerInRange)
    {
        ShowInteractionPrompt(true);
    }
}

void AGameConditionManager::OnUnhighlight_Implementation()
{
    UpdateVisualFeedback(); // This will restore the appropriate material
    
    if (!bPlayerInRange)
    {
        ShowInteractionPrompt(false);
    }
}

void AGameConditionManager::Interact_Implementation(AActor* Interactor)
{
    if (!bCanInteract)
    {
        // Play denied sound if conditions aren't met
        if (DeniedSound)
        {
            UGameplayStatics::PlaySound2D(GetWorld(), DeniedSound);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("GameConditionManager: Interaction denied - conditions not met"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("GameConditionManager: Interaction successful - changing level"));
    ChangeToNextLevel();
}

bool AGameConditionManager::CanInteract_Implementation() const
{
    return bCanInteract;
}

FText AGameConditionManager::GetInteractionText_Implementation() const
{
    if (bAllPuzzlesSolved)
    {
        return InteractionText;
    }
    else
    {
        return PuzzlesIncompleteText;
    }
}

void AGameConditionManager::FindPuzzleManager()
{
    // Find puzzle manager in the level
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APuzzleManager::StaticClass(), FoundActors);
    
    if (FoundActors.Num() > 0)
    {
        PuzzleManagerRef = Cast<APuzzleManager>(FoundActors[0]);
        UE_LOG(LogTemp, Warning, TEXT("GameConditionManager: Found PuzzleManager"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("GameConditionManager: No PuzzleManager found in level"));
    }
}

void AGameConditionManager::UpdateVisualFeedback()
{
    if (!MeshComponent)
    {
        return;
    }
    
    if (bAllPuzzlesSolved && AvailableMaterial)
    {
        MeshComponent->SetMaterial(0, AvailableMaterial);
    }
    else if (!bAllPuzzlesSolved && UnavailableMaterial)
    {
        MeshComponent->SetMaterial(0, UnavailableMaterial);
    }
    else if (OriginalMaterial)
    {
        MeshComponent->SetMaterial(0, OriginalMaterial);
    }
}