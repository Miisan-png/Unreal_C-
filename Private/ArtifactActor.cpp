#include "ArtifactActor.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "UObject/ConstructorHelpers.h"

AArtifactActor::AArtifactActor()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshFinder.Succeeded())
    {
        MeshComponent->SetStaticMesh(CubeMeshFinder.Object);
        MeshComponent->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
    }

    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(200.0f);
    InteractionSphere->SetCollisionProfileName(TEXT("Trigger"));

    ArtifactTitle = FText::FromString(TEXT("Ancient Relic"));
    ArtifactText = FText::FromString(TEXT("This is an ancient artifact with mysterious properties."));
    InteractionText = FText::FromString(TEXT("[E] Read Artifact"));
    CloseInteractionText = FText::FromString(TEXT("[E] Close"));
    
    // Initialize all widget pointers to nullptr
    ArtifactWidget = nullptr;
    TitleLabel = nullptr;
    ContentLabel = nullptr;
    InteractionLabel = nullptr;
    Image_1 = nullptr;
    Image_0 = nullptr;
    Image = nullptr;
    OriginalMaterial = nullptr;
    
    bCanInteract = false;
    bIsWidgetOpen = false;
    bShowingInteractionOnly = false;
}

AArtifactActor::~AArtifactActor()
{
    // Only try to clean up widget if it's valid and we're not in garbage collection
    if (ArtifactWidget && IsValid(ArtifactWidget) && !IsEngineExitRequested())
    {
        if (ArtifactWidget->IsInViewport())
        {
            ArtifactWidget->RemoveFromParent();
        }
    }
    
    // Clear all widget references - don't delete them as UE handles that
    ArtifactWidget = nullptr;
    TitleLabel = nullptr;
    ContentLabel = nullptr;
    InteractionLabel = nullptr;
    Image_1 = nullptr;
    Image_0 = nullptr;
    Image = nullptr;
    OriginalMaterial = nullptr;
}

void AArtifactActor::BeginPlay()
{
    Super::BeginPlay();

    if (InteractionSphere)
    {
        InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AArtifactActor::OnInteractionSphereBeginOverlap);
        InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AArtifactActor::OnInteractionSphereEndOverlap);
    }

    if (MeshComponent && MeshComponent->GetMaterial(0))
    {
        OriginalMaterial = MeshComponent->GetMaterial(0);
    }

    if (ArtifactWidgetClass)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC)
        {
            ArtifactWidget = CreateWidget<UUserWidget>(PC, ArtifactWidgetClass);
            if (ArtifactWidget)
            {
                TitleLabel = Cast<UTextBlock>(ArtifactWidget->GetWidgetFromName(TEXT("TitleLabel")));
                ContentLabel = Cast<UTextBlock>(ArtifactWidget->GetWidgetFromName(TEXT("ContentLabel")));
                InteractionLabel = Cast<UTextBlock>(ArtifactWidget->GetWidgetFromName(TEXT("Interaction_Label")));
                Image_1 = Cast<UImage>(ArtifactWidget->GetWidgetFromName(TEXT("Image_1")));
                Image_0 = Cast<UImage>(ArtifactWidget->GetWidgetFromName(TEXT("Image_0")));
                Image = Cast<UImage>(ArtifactWidget->GetWidgetFromName(TEXT("Image")));
            }
        }
    }
}

void AArtifactActor::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
    {
        bCanInteract = true;
        ShowInteractionLabel();
    }
}

void AArtifactActor::OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
    {
        bCanInteract = false;
        HideArtifactWidget();
    }
}

void AArtifactActor::ShowInteractionLabel()
{
    if (ArtifactWidget && !bIsWidgetOpen && IsValid(ArtifactWidget))
    {
        if (TitleLabel)
        {
            TitleLabel->SetVisibility(ESlateVisibility::Hidden);
        }
        if (ContentLabel)
        {
            ContentLabel->SetVisibility(ESlateVisibility::Hidden);
        }
        if (Image_1)
        {
            Image_1->SetVisibility(ESlateVisibility::Hidden);
        }
        if (Image_0)
        {
            Image_0->SetVisibility(ESlateVisibility::Hidden);
        }
        if (Image)
        {
            Image->SetVisibility(ESlateVisibility::Hidden);
        }
        if (InteractionLabel)
        {
            InteractionLabel->SetText(InteractionText);
            InteractionLabel->SetVisibility(ESlateVisibility::Visible);
        }
        
        ArtifactWidget->AddToViewport(100);
        // Fixed: Don't set bIsWidgetOpen to true here, only bShowingInteractionOnly
        bShowingInteractionOnly = true;

        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC)
        {
            PC->SetInputMode(FInputModeGameOnly());
            PC->bShowMouseCursor = false;
        }
    }
}

void AArtifactActor::ShowFullArtifactWidget()
{
    if (ArtifactWidget && bShowingInteractionOnly && IsValid(ArtifactWidget))
    {
        if (TitleLabel)
        {
            TitleLabel->SetText(ArtifactTitle);
            TitleLabel->SetVisibility(ESlateVisibility::Visible);
        }
        if (ContentLabel)
        {
            ContentLabel->SetText(ArtifactText);
            ContentLabel->SetVisibility(ESlateVisibility::Visible);
        }
        if (Image_1)
        {
            Image_1->SetVisibility(ESlateVisibility::Visible);
        }
        if (Image_0)
        {
            Image_0->SetVisibility(ESlateVisibility::Visible);
        }
        if (Image)
        {
            Image->SetVisibility(ESlateVisibility::Visible);
        }
        if (InteractionLabel)
        {
            InteractionLabel->SetText(CloseInteractionText);
            InteractionLabel->SetVisibility(ESlateVisibility::Visible);
        }
        
        bIsWidgetOpen = true;
        bShowingInteractionOnly = false;
        
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC)
        {
            PC->SetInputMode(FInputModeGameAndUI());
            PC->bShowMouseCursor = false;
        }
    }
}

void AArtifactActor::HideArtifactWidget()
{
    if (ArtifactWidget && IsValid(ArtifactWidget) && ArtifactWidget->IsInViewport())
    {
        ArtifactWidget->RemoveFromParent();
        bIsWidgetOpen = false;
        bShowingInteractionOnly = false;

        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC)
        {
            PC->SetInputMode(FInputModeGameOnly());
            PC->bShowMouseCursor = false;
        }
    }
}

void AArtifactActor::OnHighlight_Implementation()
{
    if (HighlightMaterial && MeshComponent)
    {
        MeshComponent->SetMaterial(0, HighlightMaterial);
    }
}

void AArtifactActor::OnUnhighlight_Implementation()
{
    if (OriginalMaterial && MeshComponent)
    {
        MeshComponent->SetMaterial(0, OriginalMaterial);
    }
}

void AArtifactActor::Interact_Implementation(AActor* Interactor)
{
    if (bCanInteract)
    {
        if (bShowingInteractionOnly)
        {
            ShowFullArtifactWidget();
        }
        else if (bIsWidgetOpen && !bShowingInteractionOnly)
        {
            HideArtifactWidget();
            Destroy();
        }
    }
}

bool AArtifactActor::CanInteract_Implementation() const
{
    return bCanInteract;
}

FText AArtifactActor::GetInteractionText_Implementation() const
{
    if (bShowingInteractionOnly)
    {
        return InteractionText;
    }
    else if (bIsWidgetOpen)
    {
        return CloseInteractionText;
    }
    return FText::GetEmpty();
}