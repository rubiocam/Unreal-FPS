// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)


#include "ZombieShooterGameState.h"

#include "ZombieShooterBPLibrary.h"
#include "ZombieShooterGameMode.h"
#include "ZombieShooterPlayerController.h"
#include "Net/UnrealNetwork.h"

AZombieShooterGameState::AZombieShooterGameState()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AZombieShooterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AZombieShooterGameState, RedTeamScore);
	DOREPLIFETIME(AZombieShooterGameState, BlueTeamScore);
	// Only send this on the "initial bunch" when the client first gets this actor replicated
	DOREPLIFETIME_CONDITION(AZombieShooterGameState, WaitingToStartTime, COND_InitialOnly);
}

void AZombieShooterGameState::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();

	// If we're on the server, set the waiting to start time
	if (GetLocalRole() == ROLE_Authority)
	{
		WaitingToStartTime = GetDefaultGameMode<AZombieShooterGameMode>()->WaitingToStartDuration;
	}
}

void AZombieShooterGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetMatchState() == MatchState::WaitingToStart && PlayerArray.Num() > 0 && PlayerArray.Num() == ReadyPlayers)
	{
		WaitingToStartTime -= DeltaSeconds;
		if (WaitingToStartTime <= 0.0f)
		{
			WaitingToStartTime = 0.0f;
			if (GetLocalRole() == ROLE_Authority)
			{
				if (AZombieShooterGameMode* GameMode = UZombieShooterBPLibrary::GetZombieShooterGameMode(this))
				{
					GameMode->StartMatch();
					MulticastSendAlert("STARTING MATCH", FLinearColor::Green, 3.0);
				}
			}
		}
	}
}

void AZombieShooterGameState::MulticastSendAlert_Implementation(const FString& DisplayText, FLinearColor Color,
	float Duration)
{
	AZombieShooterPlayerController* Controller = UZombieShooterBPLibrary::GetZombieShooterController(this, 0);
	if (Controller->IsLocalController())
	{
		Controller->SendAlert(DisplayText, Color, Duration);
	}
}

void AZombieShooterGameState::ShowMessage(EZombieShooterTeam MessageTeam, const FString& Sender, const FString& Message)
{
	AZombieShooterPlayerController* Controller = UZombieShooterBPLibrary::GetZombieShooterController(this, 0);
	if (Controller->IsLocalController())
	{
		Controller->SendServerMessage(MessageTeam, Sender, Message);
	}
}

void AZombieShooterGameState::ShowSaveIndicator_Implementation()
{
	AZombieShooterPlayerController* Controller = UZombieShooterBPLibrary::GetZombieShooterController(this, 0);
	if (Controller->IsLocalController())
	{
		Controller->ShowSaveIndicator();
	}
}
