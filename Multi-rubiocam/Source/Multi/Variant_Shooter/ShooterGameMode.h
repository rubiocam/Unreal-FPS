// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShooterPlayerState.h"
#include "GameFramework/GameMode.h"
#include "ShooterGameMode.generated.h"

/**
 *  Simple GameMode for a first person shooter game
 *  Manages game UI
 *  Keeps track of team scores
 */
UCLASS(abstract)
class MULTI_API AShooterGameMode : public AGameMode
{
	GENERATED_BODY()
	
protected:
	/** Map of scores by team ID */
	TMap<uint8, int32> TeamScores;

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	

public:

	AShooterGameMode();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GenericPlayerInitialization(AController* C) override;

	int RedTeam = 0;
	int BlueTeam = 0;
	
	// Time (in seconds) before the match wills tart
	UPROPERTY(EditDefaultsOnly)
	float WaitingToStartDuration = 5.0f;

	/** Increases the score for the given team */
	void IncrementTeamScore(uint8 TeamByte);

	virtual bool ShouldSpawnAtStartSpot(AController* Player) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	virtual bool ReadyToEndMatch_Implementation() override;
	virtual void HandleMatchHasEnded() override;
	virtual void HandleMatchIsWaitingToStart() override;
	
	FLinearColor GetWinner();

	float RestartGameTimer = 5.0f;
	bool bIsGameEnded = false;
	float SaveTimer = 5.0f;
};
