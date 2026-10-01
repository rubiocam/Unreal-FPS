// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)


#include "ZombieAIController.h"
#include "ZombieAICharacter.h"
#include "Navigation/PathFollowingComponent.h"

void AZombieAIController::OnPossess(APawn* InPawn)
{

	Super::OnPossess(InPawn);
	
	OwnerCharacter = Cast<AZombieAICharacter>(InPawn);

	bool bInitializedBB = false;

	if (OwnerCharacter && OwnerCharacter->BlackBoardAsset && OwnerCharacter->BTAsset)
	{
		UBlackboardComponent* BBComponent = nullptr;
		if (UseBlackboard(OwnerCharacter->BlackBoardAsset, BBComponent))
		{
			if (HasAuthority())
            {
           		RunBehaviorTree(OwnerCharacter->BTAsset);
            }
			Blackboard = BBComponent;
			bInitializedBB = true;
		}
	}

	if (!bInitializedBB)
	{
		UE_LOG(LogTemp,  Warning, TEXT("AZombieAIController can't initialize Behavior Tree for %s"), *InPawn->GetName());
	}
	
}

void AZombieAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetPathFollowingComponent())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PathFollowing status: %d"),
			(int32)GetPathFollowingComponent()->GetStatus()
		);
	}
}

void AZombieAIController::MoveTo_Implementation(AActor* MoveTarget)
{
	MoveToActor(MoveTarget);
}

