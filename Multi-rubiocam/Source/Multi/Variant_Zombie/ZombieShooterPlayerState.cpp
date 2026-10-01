// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)


#include "ZombieShooterPlayerState.h"

#include "ZombieShooterBPLibrary.h"
#include "ZombieShooterGameState.h"
#include "Net/UnrealNetwork.h"

AZombieShooterPlayerState::AZombieShooterPlayerState()
{
	SetNetUpdateFrequency(4);

	SetScore(3);
}

void AZombieShooterPlayerState::ReceiveMessage_Implementation(EZombieShooterTeam MessageTeam, const FString& Sender,
	const FString& Message)
{
	AZombieShooterGameState* GS = UZombieShooterBPLibrary::GetZombieShooterGameState(this);
	
	for (auto& Player : GS->PlayerArray )
	{
		AZombieShooterPlayerState* PS = Cast<AZombieShooterPlayerState>(Player);
		PS->ShowMessage(MessageTeam, Sender, Message);
	}
}

void AZombieShooterPlayerState::ShowMessage_Implementation(EZombieShooterTeam MessageTeam, const FString& Sender,
	const FString& Message)
{
	AZombieShooterGameState* GS = UZombieShooterBPLibrary::GetZombieShooterGameState(this);
	GS->ShowMessage(MessageTeam, Sender, Message);
}

void AZombieShooterPlayerState::UpdateReady_Implementation(bool Checked)
{
	AZombieShooterGameState* GS = UZombieShooterBPLibrary::GetZombieShooterGameState(this);

	if (Checked)
	{
		GS->IncreaseReadyPlayers();
	}
	else
	{
		GS->DecreaseReadyPlayers();
		if (GS->ReadyPlayers < 0)
		{
			GS->ResetReadyPlayers();
		}
	}
}

void AZombieShooterPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AZombieShooterPlayerState, Team);
}
