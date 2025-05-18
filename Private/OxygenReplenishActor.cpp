#include "OxygenReplenishActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AOxygenReplenishActor::AOxygenReplenishActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
	
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->SetSphereRadius(200.0f);
	InteractionSphere->SetCollisionProfileName(TEXT("Trigger"));
	
	bCanInteract = false;
}

void AOxygenReplenishActor::BeginPlay()
{
	Super::BeginPlay();
	
	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AOxygenReplenishActor::OnInteractionSphereBeginOverlap);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AOxygenReplenishActor::OnInteractionSphereEndOverlap);
	
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerOxygenSystem::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		PlayerOxygenSystem = Cast<APlayerOxygenSystem>(FoundActors[0]);
	}
	
    // Store the original material
    if (MeshComponent->GetMaterial(0))
    {
        OriginalMaterial = MeshComponent->GetMaterial(0);
    }
    
    // Create interact widget
    if (InteractWidgetClass)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC)
        {
            InteractWidget = CreateWidget<UUserWidget>(PC, InteractWidgetClass);
            if (InteractWidget)
            {
                InteractWidget->AddToViewport(0);
                InteractWidget->SetVisibility(ESlateVisibility::Hidden);
                
                ReplenishLabel = Cast<UTextBlock>(InteractWidget->GetWidgetFromName(TEXT("Rep_Label")));
                if (ReplenishLabel)
                {
                    ReplenishLabel->SetText(FText::FromString(TEXT("[E] REPLENISH OXYGEN")));
                }
            }
        }
    }
}

void AOxygenReplenishActor::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
	{
		bCanInteract = true;
	}
}

void AOxygenReplenishActor::OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
	{
		bCanInteract = false;
        ShowInteractPrompt(false);
	}
}

void AOxygenReplenishActor::PerformInteraction(AActor* Interactor)
{
	if (bCanInteract && PlayerOxygenSystem)
	{
		PlayerOxygenSystem->AddOxygen(OxygenAmount);
		
		if (bDestroyAfterUse)
		{
            ShowInteractPrompt(false);
			Destroy();
		}
	}
}

void AOxygenReplenishActor::ShowInteractPrompt(bool bShow)
{
    if (InteractWidget)
    {
        InteractWidget->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }
}

// IInteractable interface implementations
void AOxygenReplenishActor::OnHighlight_Implementation()
{
    // Apply highlight material
    if (HighlightMaterial && MeshComponent)
    {
        MeshComponent->SetMaterial(0, HighlightMaterial);
    }
}

void AOxygenReplenishActor::OnUnhighlight_Implementation()
{
    // Restore original material
    if (OriginalMaterial && MeshComponent)
    {
        MeshComponent->SetMaterial(0, OriginalMaterial);
    }
}

void AOxygenReplenishActor::Interact_Implementation(AActor* Interactor)
{
    PerformInteraction(Interactor);
}

bool AOxygenReplenishActor::CanInteract_Implementation() const
{
    return bCanInteract;
}

FText AOxygenReplenishActor::GetInteractionText_Implementation() const
{
    return InteractionText;
}