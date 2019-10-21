// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyPickupWidget.generated.h"

/**
 * 
 */
UCLASS()
class CPPASSIGNMENT_API UMyPickupWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite)
		FString HUDText;

public:
	UFUNCTION(BlueprintCallable)
		void SetHudText(FString string) { HUDText = string; }


	
};
