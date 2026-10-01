// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)


#include "ShooterGameState.h"

#include "ShooterBPLibrary.h"
#include "ShooterGameMode.h"
#include "ShooterPlayerController.h"
#include "Net/UnrealNetwork.h"

AShooterGameState::AShooterGameState()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AShooterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterGameState, RedTeamScore);
	DOREPLIFETIME(AShooterGameState, BlueTeamScore);
	// Only send this on the "initial bunch" when the client first gets this actor replicated
	DOREPLIFETIME_CONDITION(AShooterGameState, WaitingToStartTime, COND_InitialOnly);
}

void AShooterGameState::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();

	// If we're on the server, set the waiting to start time
	if (GetLocalRole() == ROLE_Authority)
	{
		WaitingToStartTime = GetDefaultGameMode<AShooterGameMode>()->WaitingToStartDuration;
	}
}

void AShooterGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (GetMatchState() == MatchState::WaitingToStart && PlayerArray.Num() > 0 && PlayerArray.Num() == ReadyPlayers)
	{
		WaitingToStartTime -= DeltaSeconds;
		if (WaitingToStartTime <= 0.0f)
		{
			WaitingToStartTime = 0.0f;
			// On server, actually start the match!
			if (GetLocalRole() == ROLE_Authority)
			{
				if (AShooterGameMode* GameMode = UShooterBPLibrary::GetShooterGameMode(this))
				{
					GameMode->StartMatch();
					MulticastSendAlert("STARTING MATCH", FLinearColor::Green, 3.0);
				}
			}
		}
	}

	if (bIsWinnerDisplayed)
	{
		WinAlertTimer -= DeltaSeconds;
		if (WinAlertTimer <= 0)
		{
			bIsWinnerDisplayed = false;
			WinAlertTimer = 5.0f;
			MulticastHideWinnerMessage();
		}
	}


	if (bIsSpreeAlertDisplayed)
	{
		SpreeAlertTimer -= DeltaSeconds;
		if (SpreeAlertTimer <= 0)
		{
			bIsSpreeAlertDisplayed = false;
			SpreeAlertTimer = 2.5f;
			MulticastHideStreakMessage();
		}
	}

}


void AShooterGameState::MulticastHideWinnerMessage_Implementation()
{
	AShooterPlayerController* Controller = UShooterBPLibrary::GetShooterController(this, 0);
	if (Controller->IsLocalController())
	{
		Controller->HideWinner();
	}
}

void AShooterGameState::MulticastHideStreakMessage_Implementation()
{
	AShooterPlayerController* Controller = UShooterBPLibrary::GetShooterController(this, 0);
	if (Controller->IsLocalController())
	{
		Controller->HideStreakMessage();
	}
}

void AShooterGameState::MulticastStreakMessage_Implementation(class AShooterPlayerState* Killer,
                                                             class AShooterPlayerState* Killed)
{
	AShooterPlayerController* Controller = UShooterBPLibrary::GetShooterController(this, 0);
	if (Controller->IsLocalController())
	{
		Controller->SendStreakMessage(Killer, Killed);
		bIsSpreeAlertDisplayed = true;
	}
}


void AShooterGameState::MulticastSendWinnerMessage_Implementation(FLinearColor Winner)
{
	AShooterPlayerController* Controller = UShooterBPLibrary::GetShooterController(this, 0);
	if (Controller->IsLocalController())
	{
		Controller->SendWinnerMessage(Winner);
		bIsWinnerDisplayed = true;
	}
}

void AShooterGameState::MulticastSendAlert_Implementation(const FString& DisplayText, FLinearColor Color, float Duration)
{
	AShooterPlayerController* Controller = UShooterBPLibrary::GetShooterController(this, 0);
	if (Controller->IsLocalController())
	{
		Controller->SendAlert(DisplayText, Color, Duration);
	}
}

void AShooterGameState::ShowMessage(EShooterTeam MessageTeam, const FString& Sender, const FString& Message)
{
	AShooterPlayerController* Controller = UShooterBPLibrary::GetShooterController(this, 0);
	if (Controller->IsLocalController())
	{
		Controller->SendServerMessage(MessageTeam, Sender, Message);
	}
}

void AShooterGameState::ShowSaveIndicator_Implementation()
{
	AShooterPlayerController* Controller = UShooterBPLibrary::GetShooterController(this, 0);
	if (Controller->IsLocalController())
	{
		Controller->ShowSaveIndicator();
	}
	
}
