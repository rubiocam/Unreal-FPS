// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)


#include "ZombieShooterBPLibrary.h"

#include "ZombieShooterCharacter.h"
#include "ZombieShooterGameMode.h"
#include "ZombieShooterGameState.h"
#include "ZombieShooterPlayerController.h"
#include "ZombieShooterPlayerState.h"
#include "Kismet/GameplayStatics.h"

class AZombieShooterCharacter* UZombieShooterBPLibrary::GetZombieShooterCharacter(const UObject* WorldContextObject,
                                                                                  int32 PlayerIndex)
{
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(WorldContextObject, PlayerIndex);
	return Cast<AZombieShooterCharacter>(Player);
}

class AZombieShooterPlayerController* UZombieShooterBPLibrary::GetZombieShooterController(
	const UObject* WorldContextObject, int32 PlayerIndex)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, PlayerIndex);
	return Cast<AZombieShooterPlayerController>(PlayerController);
}

class AZombieShooterGameMode* UZombieShooterBPLibrary::GetZombieShooterGameMode(const UObject* WorldContextObject)
{
	AGameModeBase* GameModeBase = UGameplayStatics::GetGameMode(WorldContextObject);
	return Cast<AZombieShooterGameMode>(GameModeBase);
}

class AZombieShooterGameState* UZombieShooterBPLibrary::GetZombieShooterGameState(const UObject* WorldContextObject)
{
	AGameStateBase* GameStateBase = UGameplayStatics::GetGameState(WorldContextObject);
	return Cast<AZombieShooterGameState>(GameStateBase);
}

class AZombieShooterPlayerState* UZombieShooterBPLibrary::GetZombieShooterPlayerState(const UObject* WorldContextObject,
	int32 PlayerIndex)
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(WorldContextObject, PlayerIndex);
	return Cast<AZombieShooterPlayerState>(PlayerState);
}
