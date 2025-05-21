#include "MyFPSCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "ItemManager.h"
#include "MyFPSHUD.h"
#include "ItemNameWidget.h"
#include "Components/TextBlock.h"
#include "PickableItem.h"
#include "OxygenReplenishActor.h"

AMyFPSCharacter::AMyFPSCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
    FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
    FirstPersonCamera->bUsePawnControlRotation = true;

    ItemHoldPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ItemHoldPoint"));
    ItemHoldPoint->SetupAttachment(FirstPersonCamera);
    ItemHoldPoint->SetRelativeLocation(FVector(50.0f, 20.0f, -10.0f));

    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->JumpZVelocity = 400.f;
    GetCharacterMovement()->AirControl = 0.2f;

    BreathingPhase = 0.0f;
    LastZVelocity = 0.0f;
    bWasInAir = false;
}

void AMyFPSCharacter::BeginPlay()
{
    Super::BeginPlay();

    GetCharacterMovement()->bEnablePhysicsInteraction = bEnablePhysicsInteraction;
    GetCharacterMovement()->PushForceFactor = PushForceFactor;
    GetCharacterMovement()->PushForcePointZOffsetFactor = PushForcePointZOffset;

    InitialCameraRotation = FirstPersonCamera->GetRelativeRotation();
    InitialCameraLocation = FirstPersonCamera->GetRelativeLocation();
    CurrentRotationalSway = FRotator::ZeroRotator;
    CurrentPositionalSway = FVector::ZeroVector;
    DefaultMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

    UE_LOG(LogTemp, Warning, TEXT("MyFPSCharacter::BeginPlay - Creating item name widget"));

    if (ItemNameWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("ItemNameWidgetClass is valid"));

        APlayerController* PC = Cast<APlayerController>(GetController());
        if (PC)
        {
            UE_LOG(LogTemp, Warning, TEXT("PlayerController found"));

            ItemNameWidget = CreateWidget<UItemNameWidget>(PC, ItemNameWidgetClass);
            if (ItemNameWidget)
            {
                UE_LOG(LogTemp, Warning, TEXT("ItemNameWidget created successfully"));
                ItemNameWidget->AddToViewport();

                UTextBlock* ItemNameLabel = ItemNameWidget->GetItemNameLabel();
                if (ItemNameLabel)
                {
                    UE_LOG(LogTemp, Warning, TEXT("ItemNameLabel found in widget"));

                    TArray<AActor*> FoundActors;
                    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AItemManager::StaticClass(), FoundActors);
                    if (FoundActors.Num() > 0)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Found %d ItemManager actors"), FoundActors.Num());

                        ItemManagerRef = Cast<AItemManager>(FoundActors[0]);
                        if (ItemManagerRef)
                        {
                            UE_LOG(LogTemp, Warning, TEXT("ItemManagerRef valid, registering player and UI"));

                            ItemManagerRef->RegisterPlayerRaycast(this);
                            ItemManagerRef->RegisterPickupOrigin(ItemHoldPoint);
                            ItemManagerRef->RegisterItemNameWidget(ItemNameWidget, ItemNameLabel);
                        }
                        else
                        {
                            UE_LOG(LogTemp, Error, TEXT("Failed to cast to ItemManager!"));
                        }
                    }
                    else
                    {
                        UE_LOG(LogTemp, Error, TEXT("No ItemManager actors found in level!"));
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("Failed to find ItemNameLabel in widget!"));
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to create ItemNameWidget from class!"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("PlayerController is NULL!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ItemNameWidgetClass not set! Make sure to assign it in the editor."));

        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AItemManager::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            ItemManagerRef = Cast<AItemManager>(FoundActors[0]);
            if (ItemManagerRef)
            {
                ItemManagerRef->RegisterPlayerRaycast(this);
                ItemManagerRef->RegisterPickupOrigin(ItemHoldPoint);
            }
        }
    }

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AInteractManager::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        InteractManagerRef = Cast<AInteractManager>(FoundActors[0]);
        if (InteractManagerRef)
        {
            InteractManagerRef->RegisterPlayerCharacter(this);
            InteractManagerRef->RegisterPlayerController(Cast<APlayerController>(GetController()));
        }
    }
}

void AMyFPSCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    HandleCameraSway(DeltaTime);

    PerformCameraRaycast();
}

void AMyFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &AMyFPSCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AMyFPSCharacter::MoveRight);

    PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

    PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMyFPSCharacter::StartSprint);
    PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMyFPSCharacter::StopSprint);

    PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMyFPSCharacter::StartInteract);
    PlayerInputComponent->BindAction("Interact", IE_Released, this, &AMyFPSCharacter::StopInteract);
}

void AMyFPSCharacter::MoveForward(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}


void AMyFPSCharacter::MoveRight(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
    }
}

void AMyFPSCharacter::StartSprint()
{
    bIsSprinting = true;
    GetCharacterMovement()->MaxWalkSpeed = DefaultMaxWalkSpeed * SprintSpeedMultiplier;
}

void AMyFPSCharacter::StopSprint()
{
    bIsSprinting = false;
    GetCharacterMovement()->MaxWalkSpeed = DefaultMaxWalkSpeed;
}

void AMyFPSCharacter::HandleCameraSway(float DeltaTime)
{
    float ForwardSpeed = FVector::DotProduct(GetVelocity(), GetActorForwardVector());
    float RightSpeed = FVector::DotProduct(GetVelocity(), GetActorRightVector());
    float VerticalSpeed = GetVelocity().Z;

    bool bIsInAir = GetCharacterMovement()->IsFalling();
    bool bJustLanded = bWasInAir && !bIsInAir;
    bWasInAir = bIsInAir;

    if (bJustLanded)
    {
        float ImpactStrength = FMath::Abs(LastZVelocity) / 500.0f;
        CurrentPositionalSway.Z -= LandingBobAmount * ImpactStrength;
    }

    LastZVelocity = VerticalSpeed;

    FRotator TargetRotationalSway;
    TargetRotationalSway.Pitch = -RightSpeed * 0.01f * RotationalSwayAmount;
    TargetRotationalSway.Yaw = ForwardSpeed * 0.005f * RotationalSwayAmount;
    TargetRotationalSway.Roll = RightSpeed * 0.015f * RotationalSwayAmount;

    FVector TargetPositionalSway = FVector::ZeroVector;

    if (!bIsInAir)
    {
        float SpeedFactor = GetVelocity().Size() / 500.0f;

        float BobFrequency = SpeedFactor * 5.0f;

        float TimeSeconds = GetWorld()->GetTimeSeconds();
        TargetPositionalSway.Y = FMath::Sin(TimeSeconds * BobFrequency) * PositionalSwayAmount * SpeedFactor;
        TargetPositionalSway.Z = FMath::Sin(TimeSeconds * BobFrequency * 2.0f) * PositionalSwayAmount * 0.5f * SpeedFactor;
    }

    BreathingPhase += DeltaTime * BreathingSpeed;
    if (BreathingPhase > 2.0f * PI)
    {
        BreathingPhase -= 2.0f * PI;
    }

    TargetRotationalSway.Pitch += FMath::Sin(BreathingPhase) * BreathingAmount;
    TargetPositionalSway.Z += FMath::Sin(BreathingPhase * 0.5f) * BreathingAmount * 1.5f;

    if (bIsSprinting && !bIsInAir && GetVelocity().SizeSquared() > 100.0f)
    {
        TargetRotationalSway.Pitch -= 2.0f;

        TargetPositionalSway *= 1.5f;
    }

    CurrentRotationalSway = FMath::RInterpTo(CurrentRotationalSway, TargetRotationalSway, DeltaTime, SwaySpeed);
    CurrentPositionalSway = FMath::VInterpTo(CurrentPositionalSway, TargetPositionalSway, DeltaTime, SwaySpeed);

    if (GetVelocity().IsNearlyZero() && !bIsInAir)
    {
        CurrentRotationalSway = FMath::RInterpTo(CurrentRotationalSway, FRotator::ZeroRotator, DeltaTime, ReturnSpeed);
        CurrentPositionalSway = FMath::VInterpTo(CurrentPositionalSway, FVector::ZeroVector, DeltaTime, ReturnSpeed);
    }

    FirstPersonCamera->SetRelativeRotation(InitialCameraRotation + CurrentRotationalSway);
    FirstPersonCamera->SetRelativeLocation(InitialCameraLocation + CurrentPositionalSway);
}

void AMyFPSCharacter::PerformCameraRaycast()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
        return;

    UE_LOG(LogTemp, Display, TEXT("Character performing raycast..."));

    FVector CameraLocation;
    FRotator CameraRotation;
    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

    FVector CameraForward = CameraRotation.Vector();
    FVector RayEnd = CameraLocation + (CameraForward * RaycastDistance);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.bTraceComplex = true;

    FHitResult HitResult;
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        CameraLocation,
        RayEnd,
        ECC_Visibility,
        QueryParams
    );

    if (bHit && HitResult.GetActor())
    {
        UE_LOG(LogTemp, Display, TEXT("Character raycast hit: %s"), *HitResult.GetActor()->GetName());
    }

    bool bLookingAtInteractable = false;
    if (bHit && HitResult.GetActor() && ItemManagerRef)
    {
        bLookingAtInteractable = ItemManagerRef->IsLookingAtItem();

        UE_LOG(LogTemp, Display, TEXT("Looking at interactable: %s"),
               bLookingAtInteractable ? TEXT("YES") : TEXT("NO"));
    }

    if (PC)
    {
        AMyFPSHUD* MyHUD = Cast<AMyFPSHUD>(PC->GetHUD());
        if (MyHUD)
        {
            MyHUD->SetLookingAtInteractable(bLookingAtInteractable);
        }
    }

    if (bShowDebugRaycast)
    {
        if (bHit)
        {
            DrawDebugLine(
                GetWorld(),
                CameraLocation,
                HitResult.ImpactPoint,
                RaycastHitColor,
                false,
                RaycastDebugDuration,
                0,
                2.0f
            );

            DrawDebugSphere(
                GetWorld(),
                HitResult.ImpactPoint,
                10.0f,
                16,
                HitPointColor,
                false,
                RaycastDebugDuration,
                0,
                1.0f
            );
        }
        else
        {
            DrawDebugLine(
                GetWorld(),
                CameraLocation,
                RayEnd,
                RaycastMissColor,
                false,
                RaycastDebugDuration,
                0,
                2.0f
            );
        }
    }

    if (bShowDebugRaycast && bHit && HitResult.GetActor())
    {
        UE_LOG(LogTemp, Display, TEXT("Raycast hit: %s"), *HitResult.GetActor()->GetName());
    }
}


void AMyFPSCharacter::StartInteract()
{
    if (ItemManagerRef && ItemManagerRef->IsLookingAtItem())
    {
        ItemManagerRef->PickupItem();
    }
    else if (ItemManagerRef && ItemManagerRef->IsHoldingItem())
    {
        ItemManagerRef->DropItem();
    }
    else if (InteractManagerRef)
    {
        InteractManagerRef->StartInteract();
    }
}

void AMyFPSCharacter::StopInteract()
{
    if (InteractManagerRef)
    {
        InteractManagerRef->StopInteract();
    }
}
