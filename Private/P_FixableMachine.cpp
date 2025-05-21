#include "P_FixableMachine.h"
#include "Components/StaticMeshComponent.h"
#include "PuzzleManager.h"
#include "UObject/ConstructorHelpers.h"

AP_FixableMachine::AP_FixableMachine()
{
    PrimaryActorTick.bCanEverTick = true;

    MachineMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MachineMesh"));
    RootComponent = MachineMesh;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshFinder.Succeeded())
    {
        MachineMesh->SetStaticMesh(CubeMeshFinder.Object);
        MachineMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
    }

    bIsFixed = false;
    bIsBeingFixed = false;
    FixingProgress = 0.0f;
}

void AP_FixableMachine::BeginPlay()
{
    Super::BeginPlay();

    if (!BrokenMaterial)
    {
        BrokenMaterial = MachineMesh->GetMaterial(0);
    }

    if (BrokenMaterial)
    {
        MachineMesh->SetMaterial(0, BrokenMaterial);
    }
}

void AP_FixableMachine::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsFixed)
        return;

    if (bIsBeingFixed)
    {
        UpdateFixingProgress(DeltaTime);
    }
    else if (FixingProgress > 0.0f)
    {
        FixingProgress = FMath::Max(0.0f, FixingProgress - (DeltaTime / TimeToFix) * ProgressDecayRate);
        
        if (PuzzleManagerRef)
        {
            PuzzleManagerRef->UpdateFixingProgress(FixingProgress);
        }
    }
}

void AP_FixableMachine::OnHighlight_Implementation()
{
    if (!bIsFixed)
    {
        if (HighlightMaterial)
        {
            MachineMesh->SetMaterial(0, HighlightMaterial);
        }
        if (PuzzleManagerRef)
        {
            PuzzleManagerRef->ShowInteractionUI(true, InteractionText);
        }
    }
    else
    {
        if (PuzzleManagerRef)
        {
            PuzzleManagerRef->ShowInteractionUI(false);
        }
    }
}

void AP_FixableMachine::OnUnhighlight_Implementation()
{
    if (!bIsFixed)
    {
        MachineMesh->SetMaterial(0, BrokenMaterial);
        if (PuzzleManagerRef)
        {
            PuzzleManagerRef->ShowInteractionUI(false);
        }
    }
    
    if (bIsBeingFixed)
    {
        StopFixing();
    }
}

void AP_FixableMachine::Interact_Implementation(AActor* Interactor)
{
    if (!bIsFixed && !bIsBeingFixed)
    {
        StartFixing();
    }
}

bool AP_FixableMachine::CanInteract_Implementation() const
{
    return !bIsFixed;
}

FText AP_FixableMachine::GetInteractionText_Implementation() const
{
    return InteractionText;
}

void AP_FixableMachine::StartFixing()
{
    if (!bIsFixed)
    {
        bIsBeingFixed = true;
        if (PuzzleManagerRef)
        {
            PuzzleManagerRef->OnMachineStartFixing(this);
        }
    }
}

void AP_FixableMachine::StopFixing()
{
    if (bIsBeingFixed)
    {
        bIsBeingFixed = false;
        if (PuzzleManagerRef)
        {
            PuzzleManagerRef->OnMachineStopFixing(this);
        }
    }
}

void AP_FixableMachine::UpdateFixingProgress(float DeltaTime)
{
    if (bIsBeingFixed && !bIsFixed)
    {
        FixingProgress += DeltaTime / TimeToFix;
        
        if (PuzzleManagerRef)
        {
            PuzzleManagerRef->UpdateFixingProgress(FixingProgress);
        }

        if (FixingProgress >= 1.0f)
        {
            CompleteFix();
        }
    }
}

void AP_FixableMachine::CompleteFix()
{
    bIsFixed = true;
    bIsBeingFixed = false;
    FixingProgress = 1.0f;

    if (FixedMaterial)
    {
        MachineMesh->SetMaterial(0, FixedMaterial);
    }

    if (PuzzleManagerRef)
    {
        PuzzleManagerRef->ShowInteractionUI(false);
        PuzzleManagerRef->HideProgressBar();
        PuzzleManagerRef->OnMachineFixed(this);
    }
}

void AP_FixableMachine::SetPuzzleManager(APuzzleManager* Manager)
{
    PuzzleManagerRef = Manager;
}