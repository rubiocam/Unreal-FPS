// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)


#include "ShooterPlayerState.h"

#include "ShooterBPLibrary.h"
#include "ShooterGameState.h"
#include "Net/UnrealNetwork.h"


AShooterPlayerState::AShooterPlayerState()
{
	SetNetUpdateFrequency(4);
}

void AShooterPlayerState::IncrementStreakProgress_Implementation()
{
	SpreeProgress++;
}

void AShooterPlayerState::ResetStreakProgress_Implementation()
{
	SpreeProgress = 0;
}


void AShooterPlayerState::ReceiveMessage_Implementation(EShooterTeam MessageTeam, const FString& Sender,
	const FString& Message)
{
	AShooterGameState* GS = UShooterBPLibrary::GetShooterGameState(this);
	
	for (auto& Player : GS->PlayerArray )
	{
		AShooterPlayerState* PS = Cast<AShooterPlayerState>(Player);
		if (PS->Team == MessageTeam || MessageTeam == EShooterTeam::None)
		{
			PS->ShowMessage(MessageTeam, Sender, Message);
		}
	}
}


void AShooterPlayerState::ShowMessage_Implementation(EShooterTeam MessageTeam, const FString& Sender, const FString& Message)
{
	AShooterGameState* GS = UShooterBPLibrary::GetShooterGameState(this);
	GS->ShowMessage(MessageTeam, Sender, Message);
}


void AShooterPlayerState::UpdateReady_Implementation(bool Checked)
{
	AShooterGameState* GS = UShooterBPLibrary::GetShooterGameState(this);
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

void AShooterPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterPlayerState, Team);
}
