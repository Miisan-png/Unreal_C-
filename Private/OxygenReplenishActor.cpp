#include "OxygenReplenishActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
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
	}
}

void AOxygenReplenishActor::Interact(AActor* Interactor)
{
	if (bCanInteract && PlayerOxygenSystem)
	{
		PlayerOxygenSystem->AddOxygen(OxygenAmount);
		
		if (bDestroyAfterUse)
		{
			Destroy();
		}
	}
}