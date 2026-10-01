// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)

#pragma once

#include "CoreMinimal.h"
#include "ShooterCharacter.h"
#include "GameFramework/GameState.h"
#include "GameFramework/GameStateBase.h"
#include "ShooterGameState.generated.h"

/**
 * 
 */
UCLASS()
class MULTI_API AShooterGameState : public AGameState
{
	GENERATED_BODY()

public:
	UPROPERTY(Replicated)
	int RedTeamScore = 0;

	UPROPERTY(Replicated)
	int BlueTeamScore = 0;

	UPROPERTY(Replicated)
	float WaitingToStartTime = 0.0f;

	AShooterGameState();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void HandleMatchIsWaitingToStart() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSendAlert(const FString& DisplayText, FLinearColor Color, float Duration);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSendWinnerMessage(FLinearColor Winner);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastHideWinnerMessage();
	
	//variables for the above fcn
	bool bIsWinnerDisplayed = false;
	float WinAlertTimer = 5.0f;
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastStreakMessage(class AShooterPlayerState* Killer, class AShooterPlayerState* Killed);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastHideStreakMessage();

	//variables for the above fcn
	bool bIsSpreeAlertDisplayed = false;
	float SpreeAlertTimer = 2.5f;
	
	void ShowMessage(EShooterTeam MessageTeam, const FString& Sender, const FString& Message);
	
	void IncreaseRedScore() { RedTeamScore++; }
	void IncreaseBlueScore() { BlueTeamScore++;}

	void IncreaseReadyPlayers() { ReadyPlayers++; }
	void DecreaseReadyPlayers(){ ReadyPlayers--; }
	void ResetReadyPlayers() {ReadyPlayers = 0;}

	UFUNCTION(NetMulticast, Reliable)
	void ShowSaveIndicator();
	
	UPROPERTY(Replicated)
	int ReadyPlayers = 0;
protected:
	
private:
};
