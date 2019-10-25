// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"
#include "CppAssignmentCharacter.h"
// Sets default values
APickup::APickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetAutonomousProxy(true);
}

// Called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickup::ApplyBuffs(AActor* TargetActor)
{
	ACppAssignmentCharacter* const TargetPlayer = Cast<ACppAssignmentCharacter>(TargetActor);
	auto temp = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	SetOwner(temp);

	if (Role == ROLE_Authority)
	{
		
	}
	if (Name == "Health")
	{
		TargetPlayer->ServerSetHealth(20);
		TargetPlayer->ServerSetJoy(20);
		Destroy();
	}
	else if (Name == "Stamina")
	{
		TargetPlayer->ServerSetStamina(20);
		TargetPlayer->ServerSetJoy(20);
	}
	else if (Name == "Super")
	{
		TargetPlayer->ServerSetStamina(50);
		TargetPlayer->ServerSetHealth(50);
		TargetPlayer->ServerSetJoy(50);
	}
	else if (Name == "Speed")
	{
		TargetPlayer->ServerSetSpeed(1.5);
	}
	if (Role == ROLE_Authority)
	{
		ServerDestroySelf();
	}
}

bool APickup::ServerDestroySelf_Validate()
{
	return true;
}

void APickup::ServerDestroySelf_Implementation()
{
	bool authorized = HasAuthority();
	bool didDestroy = Destroy();
}
