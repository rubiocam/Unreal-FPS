// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)


#include "BTService_MoveTowardsBase.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Variant_Zombie/ZombieAI/ZombieAICharacter.h"

UBTService_MoveTowardsBase::UBTService_MoveTowardsBase()
{
	NodeName = "MoveTowardsBase";
}

EBTNodeResult::Type UBTService_MoveTowardsBase::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent())
	{
		if (AAIController* OwnerController = OwnerComp.GetAIOwner())
		{
			if (APawn* Owner = OwnerController->GetPawn())
			{
				if (AZombieAICharacter* AIChar = Cast<AZombieAICharacter>(Owner))
				{
					if (AActor* PatrolPath = AIChar->GetBase())
					{
						MyBlackboard->SetValueAsObject("PlayerBase", PatrolPath);
						UE_LOG(LogTemp, Error, TEXT("Properly Set PlayerBase"));
						
						return EBTNodeResult::Succeeded;
					}
				}
			}
		}
	}
	
	UE_LOG(LogTemp, Error, TEXT("Failed to Set PlayerBase"));
	return EBTNodeResult::Failed;
}
