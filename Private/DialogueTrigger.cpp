#include "DialogueTrigger.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/TextBlock.h"

ADialogueTrigger::ADialogueTrigger()
{
    PrimaryActorTick.bCanEverTick = true;

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    RootComponent = TriggerBox;
    TriggerBox->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
    TriggerBox->SetCollisionProfileName(TEXT("Trigger"));

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshFinder.Succeeded())
    {
        MeshComponent->SetStaticMesh(CubeMeshFinder.Object);
        MeshComponent->SetRelativeScale3D(FVector(0.2f, 0.2f, 0.2f));
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    DialogueWidget = nullptr;
    DialogueLabel = nullptr;
    CurrentCharIndex = 0;
    TypewriterTimer = 0.0f;
    FadeTimer = 0.0f;
    DisplayTimer = 0.0f;
    CurrentText = TEXT("");
}

void ADialogueTrigger::BeginPlay()
{
    Super::BeginPlay();

    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ADialogueTrigger::OnTriggerBeginOverlap);
    TriggerBox->OnComponentEndOverlap.AddDynamic(this, &ADialogueTrigger::OnTriggerEndOverlap);

    if (DialogueWidgetClass)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC)
        {
            DialogueWidget = CreateWidget<UUserWidget>(PC, DialogueWidgetClass);
            if (DialogueWidget)
            {
                DialogueWidget->AddToViewport(1000);
                DialogueWidget->SetVisibility(ESlateVisibility::Hidden);
                
                DialogueLabel = Cast<UTextBlock>(DialogueWidget->GetWidgetFromName(TEXT("dialogue_label")));
                if (DialogueLabel)
                {
                    DialogueLabel->SetText(FText::GetEmpty());
                }
            }
        }
    }
}

void ADialogueTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsPlaying && DialogueWidget && DialogueLabel)
    {
        UpdateDialogue(DeltaTime);
    }
}

void ADialogueTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
    {
        if (!bHasBeenTriggered || bCanReplay)
        {
            StartDialogue();
        }
    }
}

void ADialogueTrigger::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
    {
        if (bIsPlaying && CurrentState != EDialogueState::FadingOut)
        {
            CurrentState = EDialogueState::FadingOut;
            FadeTimer = 0.0f;
        }
    }
}

void ADialogueTrigger::StartDialogue()
{
    if (bIsPlaying || !DialogueWidget || !DialogueLabel)
        return;

    bIsPlaying = true;
    bHasBeenTriggered = true;
    CurrentState = EDialogueState::FadingIn;
    
    CurrentText = TEXT("");
    CurrentCharIndex = 0;
    TypewriterTimer = 0.0f;
    FadeTimer = 0.0f;
    DisplayTimer = 0.0f;

    DialogueLabel->SetText(FText::GetEmpty());
    DialogueWidget->SetVisibility(ESlateVisibility::Visible);
    SetDialogueWidgetOpacity(0.0f);
}

void ADialogueTrigger::StopDialogue()
{
    if (!bIsPlaying)
        return;

    bIsPlaying = false;
    CurrentState = EDialogueState::Idle;
    
    if (DialogueWidget)
    {
        DialogueWidget->SetVisibility(ESlateVisibility::Hidden);
    }
}

void ADialogueTrigger::ResetDialogue()
{
    bHasBeenTriggered = false;
    StopDialogue();
}

void ADialogueTrigger::UpdateDialogue(float DeltaTime)
{
    switch (CurrentState)
    {
        case EDialogueState::FadingIn:
        {
            FadeTimer += DeltaTime;
            float Alpha = FMath::Clamp(FadeTimer / FadeInDuration, 0.0f, 1.0f);
            SetDialogueWidgetOpacity(Alpha);
            
            if (Alpha >= 1.0f)
            {
                CurrentState = EDialogueState::Typing;
                TypewriterTimer = 0.0f;
            }
            break;
        }

        case EDialogueState::Typing:
        {
            TypewriterTimer += DeltaTime;
            
            if (TypewriterTimer >= TypewriterSpeed)
            {
                if (CurrentCharIndex < DialogueText.Len())
                {
                    CurrentText += DialogueText[CurrentCharIndex];
                    DialogueLabel->SetText(FText::FromString(CurrentText));
                    CurrentCharIndex++;
                    TypewriterTimer = 0.0f;
                }
                else
                {
                    CurrentState = EDialogueState::Displaying;
                    DisplayTimer = 0.0f;
                }
            }
            break;
        }

        case EDialogueState::Displaying:
        {
            DisplayTimer += DeltaTime;
            
            if (DisplayTimer >= DisplayDuration)
            {
                CurrentState = EDialogueState::FadingOut;
                FadeTimer = 0.0f;
            }
            break;
        }

        case EDialogueState::FadingOut:
        {
            FadeTimer += DeltaTime;
            float Alpha = FMath::Clamp(1.0f - (FadeTimer / FadeOutDuration), 0.0f, 1.0f);
            SetDialogueWidgetOpacity(Alpha);
            
            if (Alpha <= 0.0f)
            {
                StopDialogue();
            }
            break;
        }
    }
}

void ADialogueTrigger::SetDialogueWidgetOpacity(float Opacity)
{
    if (DialogueWidget)
    {
        DialogueWidget->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, Opacity));
    }
}