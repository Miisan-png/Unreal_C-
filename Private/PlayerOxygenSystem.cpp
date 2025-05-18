// PlayerOxygenSystem.cpp
#include "PlayerOxygenSystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

APlayerOxygenSystem::APlayerOxygenSystem()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APlayerOxygenSystem::BeginPlay()
{
	Super::BeginPlay();
	
	if (HUDWidgetClass)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PC)
		{
			// First remove any existing widgets of this class
			TArray<UUserWidget*> FoundWidgets;
			for (TObjectIterator<UUserWidget> Itr; Itr; ++Itr)
			{
				if (Itr->GetClass() == HUDWidgetClass)
				{
					Itr->RemoveFromParent();
				}
			}
			
			// Now create a fresh widget
			HUDWidget = CreateWidget<UUserWidget>(PC, HUDWidgetClass);
			if (HUDWidget)
			{
				HUDWidget->AddToViewport(0);
				
				OxygenIcon = Cast<UImage>(HUDWidget->GetWidgetFromName(TEXT("Oxygen_Icon")));
				OxygenLabel = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Oxygen_Label")));
				
				// Ensure no duplicate text is showing
				if (OxygenLabel)
				{
					FString OxygenText = FString::Printf(TEXT("%d%%"), FMath::RoundToInt((CurrentOxygen / MaxOxygen) * 100.0f));
					OxygenLabel->SetText(FText::FromString(OxygenText));
				}
			}
		}
	}
}

void APlayerOxygenSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentOxygen = FMath::Clamp(CurrentOxygen - OxygenDecreaseRate * DeltaTime, 0.0f, MaxOxygen);
	
	UpdateHUD();
}

void APlayerOxygenSystem::AddOxygen(float Amount)
{
	CurrentOxygen = FMath::Clamp(CurrentOxygen + Amount, 0.0f, MaxOxygen);
	UpdateHUD();
	
}

void APlayerOxygenSystem::UpdateHUD()
{
	if (OxygenIcon && OxygenLabel)
	{
		int32 OxygenPercentage = FMath::RoundToInt((CurrentOxygen / MaxOxygen) * 100.0f);
		FString OxygenText = FString::Printf(TEXT("%d%%"), OxygenPercentage);
		
		OxygenLabel->SetText(FText::FromString(OxygenText));
		
		if (OxygenPercentage < 50)
		{
			OxygenIcon->SetColorAndOpacity(LowOxygenColor);
			OxygenLabel->SetColorAndOpacity(LowOxygenColor);
		}
		else
		{
			OxygenIcon->SetColorAndOpacity(NormalColor);
			OxygenLabel->SetColorAndOpacity(NormalColor);
		}
	}
}