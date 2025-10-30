// Copyright Epic Games, Inc. All Rights Reserved.

#include "PuzzlePiedraGameMode.h"
#include "PuzzlePiedraPlayerController.h"
#include "PuzzlePiedraPawn.h"

APuzzlePiedraGameMode::APuzzlePiedraGameMode()
{
	// no pawn by default
	DefaultPawnClass = APuzzlePiedraPawn::StaticClass();
	// use our own player controller class
	PlayerControllerClass = APuzzlePiedraPlayerController::StaticClass();
}
