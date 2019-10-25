// Fill out your copyright notice in the Description page of Project Settings.


#include "GasCloud.h"
#include "Engine/Engine.h"

// Sets default values
AGasCloud::AGasCloud()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetAutonomousProxy(true);

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
	
	for (ACppAssignmentCharacter* Target: PlayerTargets)
	{
		if (Role == ROLE_Authority) {
			Target->ServerSetHealth(-0.02);
		}
	}
}
	
