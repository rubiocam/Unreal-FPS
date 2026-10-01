// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTService_MoveTowardsBase.generated.h"

/**
 * 
 */
UCLASS()
class MULTI_API UBTService_MoveTowardsBase : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTService_MoveTowardsBase();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
};
