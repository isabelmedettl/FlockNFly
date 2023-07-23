// Copyright Epic Games, Inc. All Rights Reserved.

#include "FlockNFlyGameMode.h"
#include "FlockNFlyCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFlockNFlyGameMode::AFlockNFlyGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
