// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ZombieAIController.generated.h"

/**
 * 
 */
UCLASS()
class MULTI_API AZombieAIController : public AAIController
{
	GENERATED_BODY()

public:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;
	
	UFUNCTION(NetMulticast, Reliable)
	void MoveTo(AActor* MoveTarget);

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<class AZombieAICharacter> OwnerCharacter;
	
};
