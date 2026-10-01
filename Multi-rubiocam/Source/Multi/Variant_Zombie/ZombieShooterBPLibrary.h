// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ZombieShooterBPLibrary.generated.h"

/**
 * 
 */
UCLASS()
class MULTI_API UZombieShooterBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure)
	static class AZombieShooterCharacter* GetZombieShooterCharacter(const UObject* WorldContextObject, int32 PlayerIndex);

	UFUNCTION(BlueprintPure)
	static class AZombieShooterPlayerController* GetZombieShooterController(const UObject* WorldContextObject, int32 PlayerIndex);

	UFUNCTION(BlueprintPure)
	static class AZombieShooterGameMode* GetZombieShooterGameMode(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintPure)
	static class AZombieShooterGameState* GetZombieShooterGameState(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintPure)
	static class AZombieShooterPlayerState* GetZombieShooterPlayerState(const UObject* WorldContextObject, int32 PlayerIndex);
};
