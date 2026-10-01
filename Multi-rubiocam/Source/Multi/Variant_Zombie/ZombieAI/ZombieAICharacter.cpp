// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)


#include "ZombieAICharacter.h"

#include "ZombieAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Variant_Zombie/ZombieShooterBPLibrary.h"
#include "Variant_Zombie/ZombieShooterGameState.h"

AZombieAICharacter::AZombieAICharacter()
{
	AIControllerClass = AZombieAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AZombieAICharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentHP = MaxHP;
}

void AZombieAICharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AZombieAICharacter, CurrentHP);
}

void AZombieAICharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AZombieAICharacter::MulticastOnDeath_Implementation()
{
	// call the BP handler
	BP_OnDeath();
}

void AZombieAICharacter::SetHealth_Implementation(float Damage)
{
	CurrentHP -= Damage;
}

float AZombieAICharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float DamageApplied = FMath::Clamp(DamageAmount, 0.f, CurrentHP);

	CurrentHP -= DamageApplied;

	if (CurrentHP <= 0.f)
	{
		Die(EventInstigator);
		
		//Increase the GameState score
		if (!bIsDead)
		{
			if (AZombieShooterGameState* GameState = UZombieShooterBPLibrary::GetZombieShooterGameState(this))
			{
				//updates appropriate player score
				if (AZombieShooterCharacter* ZAttacker = Cast<AZombieShooterCharacter>(DamageCauser))
				{
					if (ZAttacker->GetTeam() == EZombieShooterTeam::Blue)
					{
						GameState->IncreaseBlueScore();
					}
					else
					{
						GameState->IncreaseRedScore();
					}
				}
			}
		}
		
		bIsDead = true;
	}
	
	return DamageApplied;
}

void AZombieAICharacter::OnRep_CurrentHP()
{
	OnDamaged.Broadcast(FMath::Max(0.0f, CurrentHP / MaxHP));
}

void AZombieAICharacter::OnRep_Team()
{
	BP_OnTeamSet();
}

void AZombieAICharacter::Die(AController* Killer)
{
	// stop character movement
	GetCharacterMovement()->StopMovementImmediately();
	
	MulticastOnDeath();
	
	SetLifeSpan(5.0f);
}

void AZombieAICharacter::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (AZombieShooterCharacter* ZChar = Cast<AZombieShooterCharacter>(OtherActor))
	{
		UGameplayStatics::ApplyDamage(OtherActor, 5.0f, GetController(), this, ZombieDamageType);
	}
	
}

void AZombieAICharacter::FindPlayer_Implementation()
{
	AZombieShooterGameState* GS = UZombieShooterBPLibrary::GetZombieShooterGameState(this);
	for (auto& Player : GS->PlayerArray )
	{
		if (const AAIController* OwnerController = GetController<AZombieAIController>())
		{
			AZombieShooterCharacter* Char = Cast<AZombieShooterCharacter>(Player->GetPlayerController()->GetPawn());
					
			FVector AItoPlayer = Char->GetActorLocation() - Owner->GetActorLocation();
			float Distance = AItoPlayer.Length();

			AItoPlayer.Normalize();
			float Angle = FMath::Acos(FVector::DotProduct(AItoPlayer, Owner->GetActorForwardVector()));

			FHitResult HitResult;
			GetWorld()->LineTraceSingleByChannel(HitResult, Owner->GetActorLocation(), Char->GetActorLocation(), ECC_Camera);

			if (Distance < 100.0f && Angle < 0.52)
			{
				if (HitResult.GetActor() == Char)
				{
					ClosestPlayer = Char;
					return;
					
				}
			}
		}
	}
}
