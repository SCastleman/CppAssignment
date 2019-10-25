// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CppAssignmentCharacter.h"
#include "CppAssignmentProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId
#include "Engine/Engine.h"
#include "GasCloud.h"
#include "UnrealNetwork.h"
#include <iostream>

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// ACppAssignmentCharacter

ACppAssignmentCharacter::ACppAssignmentCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	// Create a gun and attach it to the right-hand VR controller.
	// Create a gun mesh component
	VR_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
	VR_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	VR_Gun->bCastDynamicShadow = false;
	VR_Gun->CastShadow = false;
	VR_Gun->SetupAttachment(R_MotionController);
	VR_Gun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	VR_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
	VR_MuzzleLocation->SetupAttachment(VR_Gun);
	VR_MuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
	VR_MuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		// Counteract the rotation of the VR gun model.

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;
	//
	SetActorTickEnabled(true);
	PrimaryActorTick.bCanEverTick = true;
}

void ACppAssignmentCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	CallMyTrace();
	if (GetVelocity() != FVector(0,0,0) && !IsOutOfStamina)
	{
		Stamina -= 0.02;
		if (Stamina <= 0)
		{
			IsOutOfStamina = true;
			SpeedFramesRemaining = 0;
			CurrentWalkSpeed = DefaultSpeed * 0.8;
			GetCharacterMovement()->MaxWalkSpeed = CurrentWalkSpeed;
		}
	}
	if (SpeedFramesRemaining > 0)
	{
		SpeedFramesRemaining -= 0;
	}
	else if (SpeedFramesRemaining == 0 && CurrentWalkSpeed > DefaultSpeed)
	{
		CurrentWalkSpeed = DefaultSpeed;
		GetCharacterMovement()->MaxWalkSpeed = CurrentWalkSpeed;
	}
	if (Joy > 0)
	{
		ServerSetJoy(-0.02);
	}
}

void ACppAssignmentCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		VR_Gun->SetHiddenInGame(false, true);
		Mesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		VR_Gun->SetHiddenInGame(true, true);
		Mesh1P->SetHiddenInGame(false, true);
	}

}

//////////////////////////////////////////////////////////////////////////
// Input

void ACppAssignmentCharacter::SetJoyBool(bool NewBool)
{
	IsOutOfJoy = NewBool;
	if (IsOutOfJoy)
	{
		GetCharacterMovement()->MaxWalkSpeed = DefaultSpeed * 0.8;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = DefaultSpeed;
	}
}

void ACppAssignmentCharacter::ServerSetHealth_Implementation(float number)
{
	auto tempRole = Role;

	Health += number;
	if (Health <= 0)
	{
		//implement game over here
	}
	else if (Health > 100)
	{
		Health = 100;
	}
}

bool ACppAssignmentCharacter::ServerSetHealth_Validate(float number)
{
	return true;
}

void ACppAssignmentCharacter::ServerSetStamina_Implementation(float number)
{
	Stamina += number;
}

bool ACppAssignmentCharacter::ServerSetStamina_Validate(float number)
{
	return true;
}

void ACppAssignmentCharacter::ServerSetJoy_Implementation(float number)
{
	Joy += number;
	if (Joy <= 0)
	{
		IsOutOfJoy = false;
	}
}

bool ACppAssignmentCharacter::ServerSetJoy_Validate(float number)
{
	return true;
}

void ACppAssignmentCharacter::ServerSetSpeed_Implementation(float number)
{
	if (number > 0)
	{
		SpeedFramesRemaining = 1800;
	}
	CurrentWalkSpeed = DefaultSpeed * number;
	GetCharacterMovement()->MaxWalkSpeed = CurrentWalkSpeed;
}
bool ACppAssignmentCharacter::ServerSetSpeed_Validate(float number)
{
	return true;
}

void ACppAssignmentCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	float MaxSpeed = GetCharacterMovement()->MaxWalkSpeed;

	DOREPLIFETIME(ACppAssignmentCharacter, Joy);
	DOREPLIFETIME(ACppAssignmentCharacter, Stamina);
	DOREPLIFETIME(ACppAssignmentCharacter, Health);
	DOREPLIFETIME(ACppAssignmentCharacter, DefaultSpeed);
	DOREPLIFETIME(ACppAssignmentCharacter, CurrentWalkSpeed);
	
}


void ACppAssignmentCharacter::UseItem()
{
	if (CurrentPickup) {
		CurrentPickup->ApplyBuffs(this);
	}
}

void ACppAssignmentCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	//Bind pickup event

	PlayerInputComponent->BindAction("Use", IE_Pressed, this, &ACppAssignmentCharacter::UseItem);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ACppAssignmentCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACppAssignmentCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ACppAssignmentCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ACppAssignmentCharacter::LookUpAtRate);
}

void ACppAssignmentCharacter::OnFire()
{
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			if (bUsingMotionControllers)
			{
				const FRotator SpawnRotation = VR_MuzzleLocation->GetComponentRotation();
				const FVector SpawnLocation = VR_MuzzleLocation->GetComponentLocation();
				World->SpawnActor<ACppAssignmentProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
			}
			else
			{
				const FRotator SpawnRotation = GetControlRotation();
				// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
				const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

				// spawn the projectile at the muzzle
				World->SpawnActor<ACppAssignmentProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
			}
		}
	}

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void ACppAssignmentCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ACppAssignmentCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void ACppAssignmentCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}


void ACppAssignmentCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ACppAssignmentCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ACppAssignmentCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ACppAssignmentCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool ACppAssignmentCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &ACppAssignmentCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &ACppAssignmentCharacter::EndTouch);

		return true;
	}
	
	return false;
}


//***************************************************************************************************
//** Trace functions - used to detect items we are looking at in the world
//***************************************************************************************************
//***************************************************************************************************

//***************************************************************************************************
//** Trace() - called by our CallMyTrace() function which sets up our parameters and passes them through
//***************************************************************************************************

bool ACppAssignmentCharacter::Trace(
	UWorld* World,
	TArray<AActor*>& ActorsToIgnore,
	const FVector& Start,
	const FVector& End,
	FHitResult& HitOut,
	ECollisionChannel CollisionChannel = ECC_Pawn,
	bool ReturnPhysMat = false
	) {
	
	// The World parameter refers to our game world (map/level) 
	// If there is no World, abort
	if (!World)
	{
		return false;
	}

	// Set up our TraceParams object
	FCollisionQueryParams TraceParams(FName(TEXT("My Trace")), true, ActorsToIgnore[0]);

	// Should we simple or complex collision?
	TraceParams.bTraceComplex = true;

	// We don't need Physics materials 
	TraceParams.bReturnPhysicalMaterial = ReturnPhysMat;

	// Add our ActorsToIgnore

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGasCloud::StaticClass(), ActorsToIgnore);
	TraceParams.AddIgnoredActors(ActorsToIgnore);


	// Force clear the HitData which contains our results
	HitOut = FHitResult(ForceInit);

	// Perform our trace
	World->LineTraceSingleByChannel
		(
			HitOut,		//result
			Start,	//start
			End, //end
			CollisionChannel, //collision channel
			TraceParams
		);

	// If we hit an actor, return trued
	return (HitOut.GetActor() != NULL);
}

//***************************************************************************************************
//** CallMyTrace() - sets up our parameters and then calls our Trace() function
//***************************************************************************************************

void ACppAssignmentCharacter::CallMyTrace()
{
	// Get the location of the camera (where we are looking from) and the direction we are looking in
	const FVector Start = FirstPersonCameraComponent->GetComponentLocation();
	const FVector ForwardVector = FirstPersonCameraComponent->GetForwardVector();

	// How for in front of our character do we want our trace to extend?
	// ForwardVector is a unit vector, so we multiply by the desired distance
	const FVector End = Start + ForwardVector * 256;

	// Force clear the HitData which contains our results
	FHitResult HitData(ForceInit);

	// What Actors do we want our trace to Ignore?
	TArray<AActor*> ActorsToIgnore;

	//Ignore the player character - so you don't hit yourself!
	ActorsToIgnore.Add(this);

	// Call our Trace() function with the paramaters we have set up
	// If it Hits anything
	if (Trace(GetWorld(), ActorsToIgnore, Start, End, HitData, ECC_Visibility, false ))
	{
		// Process our HitData
		if (HitData.GetActor())
		{

			ProcessTraceHit(HitData);

		}
		else
		{
			// The trace did not return an Actor
			// An error has occurred
			// Record a message in the error log
		}
	}
	else
	{
		// We did not hit an Actor
		//ClearPickupInfo();

	}

}

//***************************************************************************************************
//** ProcessTraceHit() - process our Trace Hit result
//***************************************************************************************************


void ACppAssignmentCharacter::ProcessTraceHit(FHitResult& HitOut)
{

	// Cast the actor to APickup
	APickup* const TestPickup = Cast<APickup>(HitOut.GetActor());

	if (TestPickup)
	{
		// Keep a pointer to the Pickup
		CurrentPickup = TestPickup;

		// Set a local variable of the PickupName for the HUD
		//UE_LOG(LogClass, Warning, TEXT("PickupName: %s"), *TestPickup->GetPickupName());
		PickupName = TestPickup->GetPickupName();
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, PickupName);

		// Set a local variable of the PickupDisplayText for the HUD
		//UE_LOG(LogClass, Warning, TEXT("PickupDisplayText: %s"), *TestPickup->GetPickupDisplayText());
		//PickupDisplayText = TestPickup->GetPickupDisplayText();
		//PickupFound = true;
	} else {
	CurrentPickup = nullptr;
	PickupName = NULL;
		
	}
}
