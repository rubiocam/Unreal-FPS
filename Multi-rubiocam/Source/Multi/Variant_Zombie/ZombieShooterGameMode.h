// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ZombieShooterGameMode.generated.h"

/**
 * 
 */
UCLASS()
class MULTI_API AZombieShooterGameMode : public AGameMode
{
	GENERATED_BODY()
	
protected:
	/** Map of scores by team ID */
	TMap<uint8, int32> TeamScores;
	
	/** Gameplay initialization */
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	

public:

	AZombieShooterGameMode();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GenericPlayerInitialization(AController* C) override;

	int Player1 = 0;
	int Player2 = 0;
	
	// Time (in seconds) before the match wills tart
	UPROPERTY(EditDefaultsOnly)
	float WaitingToStartDuration = 5.0f;

	void IncrementIndividualScore(uint8 TeamByte);
	
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	virtual bool ReadyToEndMatch_Implementation() override;
	virtual void HandleMatchHasEnded() override;
	virtual void HandleMatchIsWaitingToStart() override;
	
	float RestartGameTimer = 5.0f;
	bool bIsGameEnded = false;
	float SaveTimer = 5.0f;
};

