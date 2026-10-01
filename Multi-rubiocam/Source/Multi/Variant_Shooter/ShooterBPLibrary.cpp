// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)


#include "ShooterBPLibrary.h"

#include "ShooterCharacter.h"
#include "ShooterGameMode.h"
#include "ShooterGameState.h"
#include "ShooterPlayerController.h"
#include "Kismet/GameplayStatics.h"

class AShooterCharacter* UShooterBPLibrary::GetShooterCharacter(const UObject* WorldContextObject, int32 PlayerIndex)
{
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(WorldContextObject, PlayerIndex);
	return Cast<AShooterCharacter>(Player);
}

class AShooterPlayerController* UShooterBPLibrary::GetShooterController(const UObject* WorldContextObject, int32 PlayerIndex)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, PlayerIndex);
	return Cast<AShooterPlayerController>(PlayerController);
}

class AShooterGameMode* UShooterBPLibrary::GetShooterGameMode(const UObject* WorldContextObject)
{
	AGameModeBase* GameModeBase = UGameplayStatics::GetGameMode(WorldContextObject);
	return Cast<AShooterGameMode>(GameModeBase);
}

class AShooterGameState* UShooterBPLibrary::GetShooterGameState(const UObject* WorldContextObject)
{
	AGameStateBase* GameStateBase = UGameplayStatics::GetGameState(WorldContextObject);
	return Cast<AShooterGameState>(GameStateBase);
}

class AShooterPlayerState* UShooterBPLibrary::GetShooterPlayerState(const UObject* WorldContextObject, int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(WorldContextObject, PlayerIndex);
	return Cast<AShooterPlayerState>(PlayerState);
}
