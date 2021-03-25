// Copyright Epic Games, Inc. All Rights Reserved.

#include "Cs193uGameMode.h"
#include "Cs193uCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACs193uGameMode::ACs193uGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
