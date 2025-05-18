// InteractManager.cpp
#include "InteractManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

AInteractManager::AInteractManager()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AInteractManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (PromptWidget)
    {
        PromptWidget->SetVisibility(ESlateVisibility::Hidden);
    }
}

void AInteractManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    PerformInteractionRaycast();
}

void AInteractManager::RegisterPlayerController(APlayerController* PC)
{
    PlayerController = PC;
}

void AInteractManager::RegisterPlayerCharacter(ACharacter* Character)
{
    PlayerCharacter = Character;
}

void AInteractManager::RegisterInteractPromptWidget(UUserWidget* Widget, UTextBlock* Text)
{
    PromptWidget = Widget;
    PromptText = Text;
    
    if (PromptWidget)
    {
        PromptWidget->SetVisibility(ESlateVisibility::Hidden);
    }
}

bool AInteractManager::IsLookingAtInteractable() const
{
    return CurrentInteractable.GetObject() != nullptr;
}

void AInteractManager::TryInteract()
{
    if (IsLookingAtInteractable())
    {
        CurrentInteractable->Execute_Interact(CurrentInteractable.GetObject(), PlayerCharacter);
    }
}

void AInteractManager::PerformInteractionRaycast()
{
    if (!PlayerController || !PlayerCharacter)
    {
        return;
    }

    FVector CameraLocation;
    FRotator CameraRotation;
    PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

    FVector CameraForward = CameraRotation.Vector();
    FVector RayEnd = CameraLocation + (CameraForward * InteractionRange);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(PlayerCharacter);
    QueryParams.bTraceComplex = true;

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        CameraLocation,
        RayEnd,
        ECC_Visibility,
        QueryParams
    );

    if (bShowDebugRaycast)
    {
        DrawDebugLine(
            GetWorld(),
            CameraLocation,
            bHit ? HitResult.ImpactPoint : RayEnd,
            bHit ? FColor::Green : FColor::Red,
            false,
            0.1f,
            0,
            1.0f
        );
    }

    // Check if we're no longer looking at the current interactable
    if (CurrentInteractable.GetObject() != nullptr && 
        (!bHit || !HitResult.GetActor() || !HitResult.GetActor()->GetClass()->ImplementsInterface(UInteractable::StaticClass())))
    {
        CurrentInteractable->Execute_OnUnhighlight(CurrentInteractable.GetObject());
        CurrentInteractable = nullptr;
        UpdatePromptVisibility(false);
    }

    // Check if we're looking at a new interactable
    if (bHit && HitResult.GetActor() && HitResult.GetActor()->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
    {
        TScriptInterface<IInteractable> NewInteractable;
        NewInteractable.SetObject(HitResult.GetActor());
        NewInteractable.SetInterface(Cast<IInteractable>(HitResult.GetActor()));

        if (NewInteractable.GetObject() != CurrentInteractable.GetObject())
        {
            // We're looking at a new interactable
            if (CurrentInteractable.GetObject() != nullptr)
            {
                CurrentInteractable->Execute_OnUnhighlight(CurrentInteractable.GetObject());
            }

            if (NewInteractable->Execute_CanInteract(NewInteractable.GetObject()))
            {
                CurrentInteractable = NewInteractable;
                CurrentInteractable->Execute_OnHighlight(CurrentInteractable.GetObject());
                UpdatePromptVisibility(true, CurrentInteractable->Execute_GetInteractionText(CurrentInteractable.GetObject()));
            }
            else
            {
                CurrentInteractable = nullptr;
                UpdatePromptVisibility(false);
            }
        }
    }
}

void AInteractManager::UpdatePromptVisibility(bool bVisible, const FText& Text)
{
    if (PromptWidget && PromptText)
    {
        if (bVisible)
        {
            PromptText->SetText(Text);
            PromptWidget->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            PromptWidget->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}