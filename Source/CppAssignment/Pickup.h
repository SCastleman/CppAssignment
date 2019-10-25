// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS()
class CPPASSIGNMENT_API APickup : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APickup();

	UPROPERTY(BlueprintReadWrite)
		FString Name;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
		FString GetPickupName() { return Name; }
	
	UFUNCTION(BlueprintCallable)
		void ApplyBuffs(AActor* TargetActor);

	UFUNCTION(Server, WithValidation, Reliable)
		void ServerDestroySelf();
	bool ServerDestroySelf_Validate();
	void ServerDestroySelf_Implementation();
	
};
