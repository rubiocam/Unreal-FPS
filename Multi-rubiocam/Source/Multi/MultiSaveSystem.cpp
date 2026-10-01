// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)


#include "MultiSaveSystem.h"
#include "MultiSaveGame.h"
#include "TagActor.h"
#include "Kismet/GameplayStatics.h"

void UMultiSaveSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	GroundTruthVer = NewObject<UMultiSaveGame>();
}

void UMultiSaveSystem::UpdateSave(struct FTagActorSaveData ToAdd)
{
	GroundTruthVer->TagActors.Add(ToAdd);

	//SaveGame(false);
}

void UMultiSaveSystem::SaveGame(bool SaveAsync)
{
	if (SaveAsync)
	{
		UGameplayStatics::AsyncSaveGameToSlot(GroundTruthVer, "SaveGame", 0);
	}
	else
	{
		UGameplayStatics::SaveGameToSlot(GroundTruthVer, "SaveGame", 0);
	}
}

void UMultiSaveSystem::LoadGameFromSlot()
{
	if (USaveGame* Game = UGameplayStatics::LoadGameFromSlot("SaveGame", 0))
	{
		if (UMultiSaveGame* SavedGame = Cast<UMultiSaveGame>(Game))
		{
			GroundTruthVer = SavedGame;

			for (auto TagActor : SavedGame->TagActors)
			{
				FTransform Transform = TagActor.Transform;
				ATagActor* ToPlace =  GetWorld()->SpawnActor<ATagActor>(TagActor.ClassType,Transform.GetLocation(), Transform.Rotator());
				ToPlace->SetActorRotation(Transform.GetRotation());
				ToPlace->SetActorScale3D(Transform.GetScale3D());
			}
		}
	}
	else
	{
		GroundTruthVer = NewObject<UMultiSaveGame>();
	}
}

void UMultiSaveSystem::NewGame()
{
	GroundTruthVer = NewObject<UMultiSaveGame>();
	SaveGame(false);
}

bool UMultiSaveSystem::SaveGameExists()
{
	return UGameplayStatics::DoesSaveGameExist("SaveGame", 0);
}

