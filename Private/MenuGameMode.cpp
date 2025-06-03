#include "MenuGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"

AMenuGameMode::AMenuGameMode()
{
	MenuCamera = nullptr;
}

void AMenuGameMode::BeginPlay()
{
	Super::BeginPlay();
    
	if (MenuCamera)
	{
		SetMenuCamera();
	}
	else
	{
		FindAndSetMenuCamera();
	}
}

void AMenuGameMode::SetMenuCamera()
{
	if (MenuCamera)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PC)
		{
			PC->SetViewTarget(MenuCamera);
            
			PC->SetInputMode(FInputModeUIOnly());
			PC->bShowMouseCursor = true;
		}
	}
}

void AMenuGameMode::FindAndSetMenuCamera()
{
	// Find camera by name
	AActor* FoundCamera = UGameplayStatics::GetActorOfClass(GetWorld(), ACameraActor::StaticClass());
	if (FoundCamera)
	{
		MenuCamera = Cast<ACameraActor>(FoundCamera);
		SetMenuCamera();
	}
}