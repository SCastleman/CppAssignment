// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CppAssignmentGameMode.h"
#include "CppAssignmentHUD.h"
#include "CppAssignmentCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACppAssignmentGameMode::ACppAssignmentGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ACppAssignmentHUD::StaticClass();
}
