// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)

#pragma once

#include "CoreMinimal.h"
#include "ZombieShooterPlayerState.h"
#include "GameFramework/GameState.h"
#include "ZombieShooterGameState.generated.h"

/**
 * 
 */
UCLASS()
class MULTI_API AZombieShooterGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Replicated)
	int RedTeamScore = 0;

	UPROPERTY(Replicated)
	int BlueTeamScore = 0;

	UPROPERTY(Replicated)
	float WaitingToStartTime = 0.0f;
	
	UPROPERTY(Replicated)
	int NumDead = 0;

	AZombieShooterGameState();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void HandleMatchIsWaitingToStart() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSendAlert(const FString& DisplayText, FLinearColor Color, float Duration);
	
	//variables for the above fcn
	bool bIsWinnerDisplayed = false;
	float WinAlertTimer = 5.0f;

	//variables for the above fcn
	bool bIsSpreeAlertDisplayed = false;
	float SpreeAlertTimer = 2.5f;
	
	void ShowMessage(EZombieShooterTeam MessageTeam, const FString& Sender, const FString& Message);
	
	void IncreaseRedScore() { RedTeamScore++; }
	void IncreaseBlueScore() { BlueTeamScore++;}

	void IncreaseReadyPlayers() { ReadyPlayers++; }
	void DecreaseReadyPlayers(){ ReadyPlayers--; }
	void ResetReadyPlayers() {ReadyPlayers = 0;}

	void IncreaseNumDead() { NumDead++; }
	void DecreaseNumDead() { NumDead--; }
	
	UFUNCTION(NetMulticast, Reliable)
	void ShowSaveIndicator();
	
	UPROPERTY(Replicated)
	int ReadyPlayers = 0;
protected:
	
private:
};
