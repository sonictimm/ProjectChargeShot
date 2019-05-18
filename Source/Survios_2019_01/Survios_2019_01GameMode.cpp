// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "Survios_2019_01GameMode.h"
#include "Survios_2019_01Character.h"
#include "UObject/ConstructorHelpers.h"

ASurvios_2019_01GameMode::ASurvios_2019_01GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/BP/PlayerCharacter01"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
