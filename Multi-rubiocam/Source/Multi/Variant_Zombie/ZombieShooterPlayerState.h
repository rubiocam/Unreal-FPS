// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ZombieShooterPlayerState.generated.h"

UENUM(BlueprintType)
enum class EZombieShooterTeam : uint8
{
	/* Team not assigned yet*/
	None,
	/* On red team */
	Red,
	/* On blue team */
	Blue
};


/**
 * 
 */
UCLASS()
class MULTI_API AZombieShooterPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	AZombieShooterPlayerState();
	
	UPROPERTY(Replicated)
	EZombieShooterTeam Team = EZombieShooterTeam::None;
	
	int SpreeProgress = 0;

	UPROPERTY(EditDefaultsOnly)
	TMap<int, USoundBase*> Sounds;
	
	UFUNCTION(Server, Reliable)
	void ReceiveMessage(EZombieShooterTeam MessageTeam, const FString& Sender, const FString& Message);

	UFUNCTION(Client, Reliable)
	void ShowMessage(EZombieShooterTeam MessageTeam, const FString& Sender, const FString& Message);
	
	UFUNCTION(Server, Reliable)
	void UpdateReady(bool Checked);
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
private:
	
};
