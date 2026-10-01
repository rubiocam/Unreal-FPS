// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/ShooterGameMode.h"

#include "EngineUtils.h"
#include "MultiSaveSystem.h"
#include "NNETypes.h"
#include "ShooterBPLibrary.h"
#include "ShooterCharacter.h"
#include "ShooterGameState.h"
#include "ShooterUI.h"
#include "Engine/PlayerStartPIE.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"


AShooterGameMode::AShooterGameMode()
{
	PlayerStateClass = AShooterPlayerState::StaticClass();
	GameStateClass = AShooterGameState::StaticClass();

	bDelayedStart = true;
}

void AShooterGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsGameEnded)
	{
		RestartGameTimer -= DeltaSeconds;
	}

	if (RestartGameTimer <= 0)
	{
		RestartGameTimer = 5;
		bIsGameEnded = false;
		RestartGame();
	}
	
	if (AShooterGameState* GS = UShooterBPLibrary::GetShooterGameState(this))
	{
		if (GS->GetMatchState() == MatchState::InProgress){
			SaveTimer -= DeltaSeconds;
			
			if (SaveTimer < 0)
			{
				UMultiSaveSystem* MSS = GetWorld()->GetGameInstance()->GetSubsystem<UMultiSaveSystem>();
				MSS->SaveGame(true);

				SaveTimer = 5.0f;
				GS->ShowSaveIndicator();
			}
		}
	}

	
}

void AShooterGameMode::GenericPlayerInitialization(AController* C)
{
	Super::GenericPlayerInitialization(C);

	AShooterPlayerState* State = C->GetPlayerState<AShooterPlayerState>();

	if(State->Team == EShooterTeam::None)
	{
		if (RedTeam <= BlueTeam)
		{
			RedTeam++;
			State->Team = EShooterTeam::Red;
		}
		else
		{
			if (BlueTeam < RedTeam)
			{
				BlueTeam++;
				State->Team = EShooterTeam::Blue;
			}
		}
	}

	C->SetPlayerState(State);
}

void AShooterGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void AShooterGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UMultiSaveSystem* MSS = GetWorld()->GetGameInstance()->GetSubsystem<UMultiSaveSystem>();
	MSS->SaveGame(false);
}

void AShooterGameMode::IncrementTeamScore(uint8 TeamByte)
{
	// retrieve the team score if any
	int32 Score = 0;
	if (int32* FoundScore = TeamScores.Find(TeamByte))
	{
		Score = *FoundScore;
	}

	// increment the score for the given team
	++Score;
	TeamScores.Add(TeamByte, Score);
}

bool AShooterGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	return false;
}

AActor* AShooterGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	// Choose a player start
	APlayerStart* FoundPlayerStart = nullptr;
	UClass* PawnClass = GetDefaultPawnClassForController(Player);
	APawn* PawnToFit = PawnClass ? PawnClass->GetDefaultObject<APawn>() : nullptr;
	TArray<APlayerStart*> UnOccupiedStartPoints;
	TArray<APlayerStart*> OccupiedStartPoints;
	TArray<APlayerStart*> BackupStartPoints;
	UWorld* World = GetWorld();

	// NEW!!!! Figure out the player's team
	EShooterTeam Team = Player->GetPlayerState<AShooterPlayerState>()->Team;

	// NEW!!!! Assign start tag based on team
	FName PlayerStartTag = NAME_None;
	switch (Team)
	{
	case EShooterTeam::None:
		PlayerStartTag = "Backup";
		break;
	case EShooterTeam::Red:
		PlayerStartTag = "Red";
		break;
	case EShooterTeam::Blue:
		PlayerStartTag = "Blue";
		break;
	}
	
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		APlayerStart* PlayerStart = *It;
		if (PlayerStart->PlayerStartTag == "Backup")
		{
			BackupStartPoints.Add(PlayerStart);	
		}
	}
	
	
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		APlayerStart* PlayerStart = *It;

		if (PlayerStart->IsA<APlayerStartPIE>())
		{
			// Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
			FoundPlayerStart = PlayerStart;
			break;
		}
		// NEW!!!! Prioritize player starts which match the team
		else if (PlayerStartTag == NAME_None || PlayerStartTag == PlayerStart->PlayerStartTag)
		{
			FVector ActorLocation = PlayerStart->GetActorLocation();
			const FRotator ActorRotation = PlayerStart->GetActorRotation();
			if (!World->EncroachingBlockingGeometry(PawnToFit, ActorLocation, ActorRotation))
			{
				UnOccupiedStartPoints.Add(PlayerStart);
			}
			else if (World->FindTeleportSpot(PawnToFit, ActorLocation, ActorRotation))
			{
				OccupiedStartPoints.Add(PlayerStart);
			}
		}
	}

	for (TActorIterator<AShooterCharacter> It(World); It; ++It)
	{
		AShooterCharacter* ShooterChar = *It;
		FVector CharLoc = ShooterChar->GetActorLocation();

		if (ShooterChar->GetTeam() != Team)
		{
			OccupiedStartPoints.RemoveAll([&](const APlayerStart* Start)
			{
				FVector StartToChar = CharLoc - Start->GetActorLocation(); 
				return StartToChar.Length() < 1000.0f;
			});

			UnOccupiedStartPoints.RemoveAll([&](const APlayerStart* Start)
			{
				FVector StartToChar = CharLoc - Start->GetActorLocation(); 
				return StartToChar.Length() < 1000.0f;
			});
		}
	}
	

	if (UnOccupiedStartPoints.IsEmpty() && OccupiedStartPoints.IsEmpty()) //if there are no places to even check to spawn
	{
		FoundPlayerStart = BackupStartPoints[FMath::RandRange(0, BackupStartPoints.Num() - 1)];
	}
	
	if (FoundPlayerStart == nullptr)
	{
		if (UnOccupiedStartPoints.Num() > 0)
		{
			FoundPlayerStart = UnOccupiedStartPoints[FMath::RandRange(0, UnOccupiedStartPoints.Num() - 1)];
		}
		else if (OccupiedStartPoints.Num() > 0)
		{
			FoundPlayerStart = OccupiedStartPoints[FMath::RandRange(0, OccupiedStartPoints.Num() - 1)];
		}
	}
	return FoundPlayerStart;
}

bool AShooterGameMode::ReadyToEndMatch_Implementation()
 {
	AShooterGameState* State = UShooterBPLibrary::GetShooterGameState(this);
 	return State->RedTeamScore == 10 || State->BlueTeamScore == 10 ;
 }

void AShooterGameMode::HandleMatchHasEnded()
{
	bIsGameEnded = true;
	AShooterGameState* State = UShooterBPLibrary::GetShooterGameState(this);
	State->MulticastSendWinnerMessage(GetWinner());
	
}

void AShooterGameMode::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();
	
	UMultiSaveSystem* MSS = GetWorld()->GetGameInstance()->GetSubsystem<UMultiSaveSystem>();
	MSS->LoadGameFromSlot();
}

FLinearColor AShooterGameMode::GetWinner()
{
	AShooterGameState* State = GetGameState<AShooterGameState>();
	if (State->RedTeamScore == 10)
	{
		return FLinearColor::Red;
	}
	else if (State->BlueTeamScore == 10)
	{
		return FLinearColor::Blue;
	}

	//if for some reason no one won return black
	return FLinearColor::Black;
}
