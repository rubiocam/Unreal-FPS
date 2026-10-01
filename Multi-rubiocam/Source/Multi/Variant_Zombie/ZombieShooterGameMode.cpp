// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)


#include "ZombieShooterGameMode.h"

#include "EngineUtils.h"
#include "MultiSaveSystem.h"
#include "ZombieShooterBPLibrary.h"
#include "ZombieShooterCharacter.h"
#include "ZombieShooterGameState.h"
#include "ZombieShooterPlayerState.h"
#include "Engine/PlayerStartPIE.h"


AZombieShooterGameMode::AZombieShooterGameMode()
{
	PlayerStateClass = AZombieShooterPlayerState::StaticClass();
	GameStateClass = AZombieShooterGameState::StaticClass();

	bDelayedStart = true;
}

void AZombieShooterGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void AZombieShooterGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UMultiSaveSystem* MSS = GetWorld()->GetGameInstance()->GetSubsystem<UMultiSaveSystem>();
	MSS->SaveGame(false);
}

void AZombieShooterGameMode::Tick(float DeltaSeconds)
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
	
	if (AZombieShooterGameState* GS = UZombieShooterBPLibrary::GetZombieShooterGameState(this))
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

void AZombieShooterGameMode::GenericPlayerInitialization(AController* C)
{
	Super::GenericPlayerInitialization(C);

	AZombieShooterPlayerState* State = C->GetPlayerState<AZombieShooterPlayerState>();

	if(State->Team == EZombieShooterTeam::None)
	{
		//player 1 is red & player 2 is blue
		if (Player1 <= Player2)
		{
			Player1++;
			State->Team = EZombieShooterTeam::Red;
		}
		else
		{
			if (Player2 < Player1)
			{
				Player2++;
				State->Team = EZombieShooterTeam::Blue;
			}
		}
	}

	C->SetPlayerState(State);
}

//to keep track of how many zombies each player has killed
void AZombieShooterGameMode::IncrementIndividualScore(uint8 TeamByte)
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

bool AZombieShooterGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	return false;
}

AActor* AZombieShooterGameMode::ChoosePlayerStart_Implementation(AController* Player)
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
	EZombieShooterTeam Team = Player->GetPlayerState<AZombieShooterPlayerState>()->Team;

	// NEW!!!! Assign start tag based on team
	FName PlayerStartTag = NAME_None;
	switch (Team)
	{
	case EZombieShooterTeam::None:
		PlayerStartTag = "Backup";
		break;
	case EZombieShooterTeam::Red:
		PlayerStartTag = "Red";
		break;
	case EZombieShooterTeam::Blue:
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

	for (TActorIterator<AZombieShooterCharacter> It(World); It; ++It)
	{
		AZombieShooterCharacter* ShooterChar = *It;
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

bool AZombieShooterGameMode::ReadyToEndMatch_Implementation()
{
	AZombieShooterGameState* State = UZombieShooterBPLibrary::GetZombieShooterGameState(this);
	return State->NumDead == State->PlayerArray.Num();
}

void AZombieShooterGameMode::HandleMatchHasEnded()
{
	bIsGameEnded = true;
}

void AZombieShooterGameMode::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();
	
	UMultiSaveSystem* MSS = GetWorld()->GetGameInstance()->GetSubsystem<UMultiSaveSystem>();
	MSS->LoadGameFromSlot();
}
