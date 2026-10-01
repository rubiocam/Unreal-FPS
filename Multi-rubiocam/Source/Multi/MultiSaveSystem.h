// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MultiSaveSystem.generated.h"

/**
 * 
 */
UCLASS()
class MULTI_API UMultiSaveSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<class UMultiSaveGame> GroundTruthVer;
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	void UpdateSave(struct FTagActorSaveData ToAdd);
	void SaveGame(bool SaveAsync);

	void LoadGameFromSlot();

	/*
	 * To load a new game
	 */
	UFUNCTION(BlueprintCallable)
	void NewGame();

	UFUNCTION(BlueprintCallable)
	bool SaveGameExists();
};
