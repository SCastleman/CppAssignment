// Fill out your copyright notice in the Description page of Project Settings.


#include "GasCloud.h"
#include "Engine/Engine.h"

// Sets default values
AGasCloud::AGasCloud()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGasCloud::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGasCloud::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
	for (auto It = PlayerTargets.CreateIterator(); It; ++It)
	{
		ACppAssignmentCharacter* tempChar = *It;
		tempChar->ServerSetHealth(1);
		bool tempbool = HasAuthority();
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "" + HasAuthority());
	}
}
	
