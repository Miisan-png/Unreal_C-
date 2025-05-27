// PlayerOxygenSystem.cpp
#include "PlayerOxygenSystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Components/Image.h"
#include "Engine/Engine.h"

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

	// Create screen overlay widget for red tint effect
	if (ScreenOverlayWidgetClass)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PC)
		{
			ScreenOverlayWidget = CreateWidget<UUserWidget>(PC, ScreenOverlayWidgetClass);
			if (ScreenOverlayWidget)
			{
				ScreenOverlayWidget->AddToViewport(1000); // High Z-order to appear on top
				
				// Find the screen tint image in the overlay widget
				ScreenTintImage = Cast<UImage>(ScreenOverlayWidget->GetWidgetFromName(TEXT("ScreenTint_Image")));
				
				if (ScreenTintImage)
				{
					// Initially set to transparent
					FLinearColor InitialTint = ScreenTintColor;
					InitialTint.A = 0.0f;
					ScreenTintImage->SetColorAndOpacity(InitialTint);
				}
			}
		}
	}
}

void APlayerOxygenSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Decrease oxygen over time
	CurrentOxygen = FMath::Clamp(CurrentOxygen - OxygenDecreaseRate * DeltaTime, 0.0f, MaxOxygen);
	
	// Update HUD and screen effects
	UpdateHUD();
	UpdateScreenTint();

	// Check for game over condition
	if (CurrentOxygen <= 0.0f && !bGameOverTriggered)
	{
		TriggerGameOver();
	}
}

void APlayerOxygenSystem::AddOxygen(float Amount)
{
	CurrentOxygen = FMath::Clamp(CurrentOxygen + Amount, 0.0f, MaxOxygen);
	UpdateHUD();
	UpdateScreenTint();
	
	// Reset game over state if oxygen is restored
	if (CurrentOxygen > 0.0f && bGameOverTriggered)
	{
		bGameOverTriggered = false;
		GetWorldTimerManager().ClearTimer(GameOverTimerHandle);
	}
}

void APlayerOxygenSystem::UpdateHUD()
{
	if (OxygenIcon && OxygenLabel)
	{
		int32 OxygenPercentage = FMath::RoundToInt((CurrentOxygen / MaxOxygen) * 100.0f);
		FString OxygenText = FString::Printf(TEXT("%d%%"), OxygenPercentage);
		
		OxygenLabel->SetText(FText::FromString(OxygenText));
		
		// Change color based on oxygen level
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

void APlayerOxygenSystem::UpdateScreenTint()
{
	if (ScreenTintImage)
	{
		float OxygenPercentage = GetOxygenPercentage();
		float ThresholdPercentage = LowOxygenThreshold / MaxOxygen;
		
		if (OxygenPercentage <= ThresholdPercentage)
		{
			// Calculate tint intensity based on how low oxygen is
			// When oxygen is at threshold, alpha = 0
			// When oxygen is at 0, alpha = MaxScreenTintAlpha
			float TintIntensity = (ThresholdPercentage - OxygenPercentage) / ThresholdPercentage;
			TintIntensity = FMath::Clamp(TintIntensity, 0.0f, 1.0f);
			
			// Apply easing for smoother transition
			float EasedIntensity = FMath::Pow(TintIntensity, 1.5f);
			
			FLinearColor CurrentTint = ScreenTintColor;
			CurrentTint.A = EasedIntensity * MaxScreenTintAlpha;
			
			ScreenTintImage->SetColorAndOpacity(CurrentTint);
			
			// Add slight pulsing effect when very low on oxygen
			if (OxygenPercentage < 0.1f) // Less than 10%
			{
				float PulseRate = 3.0f; // Pulses per second
				float PulseAmount = 0.2f * FMath::Sin(GetWorld()->GetTimeSeconds() * PulseRate * 2.0f * PI);
				CurrentTint.A = FMath::Clamp(CurrentTint.A + PulseAmount, 0.0f, 1.0f);
				ScreenTintImage->SetColorAndOpacity(CurrentTint);
			}
		}
		else
		{
			// No tint when oxygen is above threshold
			FLinearColor NoTint = ScreenTintColor;
			NoTint.A = 0.0f;
			ScreenTintImage->SetColorAndOpacity(NoTint);
		}
	}
}

void APlayerOxygenSystem::TriggerGameOver()
{
	if (bGameOverTriggered)
		return;
		
	bGameOverTriggered = true;
	
	UE_LOG(LogTemp, Warning, TEXT("Oxygen depleted! Game Over in %f seconds..."), GameOverDelay);
	
	// Set timer to restart the game after delay
	GetWorldTimerManager().SetTimer(
		GameOverTimerHandle,
		this,
		&APlayerOxygenSystem::RestartGame,
		GameOverDelay,
		false
	);
}

void APlayerOxygenSystem::RestartGame()
{
	UE_LOG(LogTemp, Warning, TEXT("Restarting game due to oxygen depletion..."));
	
	// Clean up widgets before restart
	if (HUDWidget)
	{
		HUDWidget->RemoveFromParent();
	}
	if (ScreenOverlayWidget)
	{
		ScreenOverlayWidget->RemoveFromParent();
	}
	
	// Restart the level
	if (RestartLevelName.IsNone())
	{
		// Restart current level by getting current level name
		FString CurrentLevelName = GetWorld()->GetMapName();
		// Remove the path and keep just the level name
		int32 LastSlash = CurrentLevelName.Find(TEXT("/"), ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		if (LastSlash != INDEX_NONE)
		{
			CurrentLevelName = CurrentLevelName.RightChop(LastSlash + 1);
		}
		UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName));
	}
	else
	{
		// Load specific level
		UGameplayStatics::OpenLevel(this, RestartLevelName);
	}
}