// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)


#include "BTService_FindClosestPlayer.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/GameMode.h"
#include "Variant_Zombie/ZombieShooterBPLibrary.h"
#include "Variant_Zombie/ZombieShooterCharacter.h"
#include "Variant_Zombie/ZombieShooterGameState.h"

UBTService_FindClosestPlayer::UBTService_FindClosestPlayer()
{
	NodeName = "FindClosestPlayer";

	//PlayerKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_FindClosestPlayer, PlayerKey), AActor::StaticClass());
}

/*
void UBTService_FindClosestPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UE_LOG(LogTemp, Warning, TEXT("TickNode from FindClosestPlayer!"));

	if (AZombieShooterGameState* ZGS = UZombieShooterBPLibrary::GetZombieShooterGameState(this))
	{
		if ((ZGS->IsMatchInProgress() || ZGS->GetMatchState() == MatchState::InProgress))
		{
			FindPlayer(&OwnerComp);
			UE_LOG(LogTemp, Error, TEXT("After Running FindPlayer"));
		}
	}
	
}
*/

void UBTService_FindClosestPlayer::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		PlayerKey.ResolveSelectedKey(*BBAsset);
	}
}

EBTNodeResult::Type UBTService_FindClosestPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	
	if (AZombieShooterGameState* ZGS = UZombieShooterBPLibrary::GetZombieShooterGameState(this))
	{
		if ((ZGS->IsMatchInProgress() || ZGS->GetMatchState() == MatchState::InProgress))
		{
			FindPlayer(&OwnerComp);
			UE_LOG(LogTemp, Error, TEXT("After Running FindPlayer"));
		}
	}

	if (UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent())
	{
		if (MyBlackboard->GetValueAsObject("ClosestPlayer") != nullptr)
		{
			return EBTNodeResult::Succeeded;
		}
		else
		{
			return EBTNodeResult::Failed;
		}
	}
	else
	{
		return EBTNodeResult::Failed;
	}
	
}

void UBTService_FindClosestPlayer::FindPlayer_Implementation(UBehaviorTreeComponent* OwnerComp)
{
	if (UBlackboardComponent* MyBlackboard = OwnerComp->GetBlackboardComponent())
	{
		AZombieShooterGameState* GS = UZombieShooterBPLibrary::GetZombieShooterGameState(this);
		for (auto& Player : GS->PlayerArray )
		{
			if (const AAIController* OwnerController = OwnerComp->GetAIOwner())
			{
				if (const AActor* Owner = OwnerController->GetPawn())
				{
					AZombieShooterCharacter* Char = Cast<AZombieShooterCharacter>(Player->GetPlayerController()->GetPawn());
					
					FVector AItoPlayer = Char->GetActorLocation() - Owner->GetActorLocation();
					float Distance = AItoPlayer.Length();

					AItoPlayer.Normalize();
					float Angle = FMath::Acos(FVector::DotProduct(AItoPlayer, Owner->GetActorForwardVector()));

					FHitResult HitResult;
					GetWorld()->LineTraceSingleByChannel(HitResult, Owner->GetActorLocation(), Char->GetActorLocation(), ECC_Camera);

					if (Distance < 500.0f)
					{
						//if (HitResult.GetActor() == Char)
						//{
							MyBlackboard->SetValueAsObject("ClosestPlayer", Char);
							AAIController* AI = OwnerComp->GetAIOwner();
							UE_LOG(LogTemp, Error, TEXT("Found the ClosestPlayer!"))
							return;
						//}
					}
				}
			}
		}

		UE_LOG(LogTemp, Error, TEXT("Failed to find FindPlayer"));
		MyBlackboard->SetValueAsObject("ClosestPlayer", nullptr);
	}
}
