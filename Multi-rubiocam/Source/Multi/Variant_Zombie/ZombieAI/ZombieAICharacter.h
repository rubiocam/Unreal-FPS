// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)

#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "Variant_Zombie/ZombieShooterCharacter.h"
#include "ZombieAICharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FZAIDamagedDelegate, float, LifePercent);
/**
 * 
 */
UCLASS()
class MULTI_API AZombieAICharacter : public AMultiCharacter
{
	GENERATED_BODY()

public:
	AZombieAICharacter();

	AActor* GetBase() {return Base;}
	virtual void BeginPlay() override;
	
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override;
	
	UFUNCTION(NetMulticast, Reliable)
	void SetHealth(float NewHP);

	float GetHealth(){return CurrentHP;}

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Health")
	float MaxHP;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentHP, BlueprintReadOnly, Category="Health")
	float CurrentHP;

	UFUNCTION()
	void OnRep_CurrentHP();

	/** Called to allow Blueprint code to react to this character's death */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta = (DisplayName = "On Death"))
	void BP_OnDeath();

	UFUNCTION()
	void OnRep_Team();

	/** Called to allow Blueprint code to react to the team being set */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta = (DisplayName = "On Team Set"))
	void BP_OnTeamSet();
	
	/** Damaged delegate */
	FZAIDamagedDelegate OnDamaged;
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnDeath();

	void Die(AController* Killer);
	
	UPROPERTY(EditAnywhere, Category = AI)
	TObjectPtr<class UBlackboardData> BlackBoardAsset;

	UPROPERTY(EditAnywhere, Category = AI)
	TObjectPtr<class UBehaviorTree> BTAsset;

	UPROPERTY(EditAnywhere, Category = AI)
	AActor* Base = nullptr; 

	AActor* ClosestPlayer = nullptr;
	
	UFUNCTION(Server, Reliable)
	void FindPlayer();

	float DelayTimer = 5.0f;
	bool bIsDelayed = false;
	bool bIsDead = false;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	UPROPERTY(EditAnywhere, Category="Projectile|Hit")
	TSubclassOf<UDamageType> ZombieDamageType;
	
	
	
	friend class AZombieAIController;
};
