// UIContainerActor.cpp
// Implementation of the UI Container Actor

#include "UIContainerActor.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AUIContainerActor::AUIContainerActor()
{
    // Set this actor to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;

    // Create a root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    SetRootComponent(RootSceneComponent);
}

// Called when the game starts or when spawned
void AUIContainerActor::BeginPlay()
{
    Super::BeginPlay();
    
    // Create all widgets from the widget classes array
    for (auto WidgetClass : WidgetClasses)
    {
        if (WidgetClass)
        {
            AddWidget(WidgetClass);
        }
    }
}

// Called when the game ends or when destroyed
void AUIContainerActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    
    // Remove all widgets when this actor is destroyed
    RemoveAllWidgets();
}

// Called every frame
void AUIContainerActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// Add a widget to this container at runtime
UUserWidget* AUIContainerActor::AddWidget(TSubclassOf<UUserWidget> WidgetClass)
{
    if (!WidgetClass)
    {
        return nullptr;
    }
    
    // Create the widget
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PlayerController)
    {
        return nullptr;
    }
    
    UUserWidget* NewWidget = CreateWidget<UUserWidget>(PlayerController, WidgetClass);
    if (NewWidget)
    {
        // Add to viewport
        NewWidget->AddToViewport();
        
        // Add to our instances array
        WidgetInstances.Add(NewWidget);
        
        return NewWidget;
    }
    
    return nullptr;
}

// Remove a widget from this container at runtime
void AUIContainerActor::RemoveWidget(UUserWidget* Widget)
{
    if (Widget)
    {
        // Remove from viewport
        Widget->RemoveFromParent();
        
        // Remove from our instances array
        WidgetInstances.Remove(Widget);
    }
}

// Remove all widgets
void AUIContainerActor::RemoveAllWidgets()
{
    for (UUserWidget* Widget : WidgetInstances)
    {
        if (Widget)
        {
            Widget->RemoveFromParent();
        }
    }
    
    WidgetInstances.Empty();
}

// Get a widget by class
UUserWidget* AUIContainerActor::GetWidgetByClass(TSubclassOf<UUserWidget> WidgetClass)
{
    if (!WidgetClass)
    {
        return nullptr;
    }
    
    for (UUserWidget* Widget : WidgetInstances)
    {
        if (Widget && Widget->IsA(WidgetClass))
        {
            return Widget;
        }
    }
    
    return nullptr;
}