#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Camera/CameraActor.h"
#include "MenuGameMode.generated.h"

UCLASS()
class FIRSTPERSONTEST_API AMenuGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AMenuGameMode();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu Camera")
    ACameraActor* MenuCamera;

    UFUNCTION(BlueprintCallable, Category = "Menu Camera")
    void SetMenuCamera();

    UFUNCTION(BlueprintCallable, Category = "Menu Camera")
    void FindAndSetMenuCamera();
};