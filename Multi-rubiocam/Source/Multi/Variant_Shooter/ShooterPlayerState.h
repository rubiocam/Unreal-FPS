// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ShooterPlayerState.generated.h"


UENUM(BlueprintType)
enum class EShooterTeam : uint8
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
class MULTI_API AShooterPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AShooterPlayerState();
	
	UPROPERTY(Replicated)
	EShooterTeam Team = EShooterTeam::None;
	
	int SpreeProgress = 0;

	UPROPERTY(EditDefaultsOnly)
	TMap<int, USoundBase*> Sounds;

	UFUNCTION(NetMulticast, Reliable)
	void IncrementStreakProgress();

	UFUNCTION(NetMulticast, Reliable)
	void ResetStreakProgress();
	
	UFUNCTION(Server, Reliable)
	void ReceiveMessage(EShooterTeam MessageTeam, const FString& Sender, const FString& Message);

	UFUNCTION(Client, Reliable)
	void ShowMessage(EShooterTeam MessageTeam, const FString& Sender, const FString& Message);
	
	UFUNCTION(Server, Reliable)
	void UpdateReady(bool Checked);
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
private:
	
};
