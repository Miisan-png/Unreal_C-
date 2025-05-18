// UIContainerActor.h
// A container actor that can hold UI widgets for your game

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blueprint/UserWidget.h"
#include "UIContainerActor.generated.h"

UCLASS()
class FIRSTPERSONTEST_API AUIContainerActor : public AActor
{
	GENERATED_BODY()
    
public:    
	// Sets default values for this actor's properties
	AUIContainerActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
    
	// Called when the game ends or when destroyed
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:    
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Component to attach UI to
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* RootSceneComponent;

	// The list of widget classes to create and add to viewport
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TArray<TSubclassOf<UUserWidget>> WidgetClasses;
    
	// The list of created widget instances (runtime)
	UPROPERTY()
	TArray<UUserWidget*> WidgetInstances;
    
	// Add a widget to this container at runtime
	UFUNCTION(BlueprintCallable, Category = "UI")
	UUserWidget* AddWidget(TSubclassOf<UUserWidget> WidgetClass);
    
	// Remove a widget from this container at runtime
	UFUNCTION(BlueprintCallable, Category = "UI")
	void RemoveWidget(UUserWidget* Widget);
    
	// Remove all widgets
	UFUNCTION(BlueprintCallable, Category = "UI")
	void RemoveAllWidgets();
    
	// Get a widget by class
	UFUNCTION(BlueprintCallable, Category = "UI")
	UUserWidget* GetWidgetByClass(TSubclassOf<UUserWidget> WidgetClass);
};