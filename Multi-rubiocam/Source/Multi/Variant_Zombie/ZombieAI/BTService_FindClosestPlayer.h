// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTService_FindClosestPlayer.generated.h"

/**
 * 
 */
UCLASS()
class MULTI_API UBTService_FindClosestPlayer : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTService_FindClosestPlayer();
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	FName GetSelectedPlayerKey() const { return PlayerKey.SelectedKeyName; }

	UFUNCTION(Server, Reliable)
	void FindPlayer(UBehaviorTreeComponent* OwnerComp);
	
	
	
private:
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector PlayerKey;
};
