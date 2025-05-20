#include "P_FixableMachine.h"
#include "Components/StaticMeshComponent.h"
#include "PuzzleManager.h"
#include "UObject/ConstructorHelpers.h"

AP_FixableMachine::AP_FixableMachine()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create and setup the cube mesh component
    MachineMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MachineMesh"));
    RootComponent = MachineMesh;

    // Load the cube static mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshFinder.Succeeded())
    {
        MachineMesh->SetStaticMesh(CubeMeshFinder.Object);
        MachineMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f)); // Make it a bit smaller
    }

    // Initialize variables
    bIsFixed = false;
    bIsBeingFixed = false;
    FixingProgress = 0.0f;
}

void AP_FixableMachine::BeginPlay()
{
    Super::BeginPlay();

    // Store the original material if not already set
    if (!BrokenMaterial)
    {
        BrokenMaterial = MachineMesh->GetMaterial(0);
    }

    // Set the initial broken material
    if (BrokenMaterial)
    {
        MachineMesh->SetMaterial(0, BrokenMaterial);
    }
}

void AP_FixableMachine::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsBeingFixed)
    {
        UpdateFixingProgress(DeltaTime);
    }
}

void AP_FixableMachine::OnHighlight_Implementation()
{
    // Only show highlight and interaction UI if the machine isn't fixed
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
}

void AP_FixableMachine::OnUnhighlight_Implementation()
{
    // Only change material back if not fixed
    if (!bIsFixed)
    {
        MachineMesh->SetMaterial(0, BrokenMaterial);
        if (PuzzleManagerRef)
        {
            PuzzleManagerRef->ShowInteractionUI(false);
        }
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
        FixingProgress = 0.0f;
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
        PuzzleManagerRef->OnMachineFixed(this);
    }
}

void AP_FixableMachine::SetPuzzleManager(APuzzleManager* Manager)
{
    PuzzleManagerRef = Manager;
}