// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)

#pragma once

#include "CoreMinimal.h"
#include "MultiCharacter.h"
#include "ShooterWeaponHolder.h"
#include "ZombieShooterCharacter.generated.h"

enum class EZombieShooterTeam : uint8;
class AShooterWeapon;
class UInputAction;
class UInputComponent;
class UPawnNoiseEmitterComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FZBulletCountUpdatedDelegate, int32, MagazineSize, int32, Bullets);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FZDamagedDelegate, float, LifePercent);

/**
 * 
 */
UCLASS()
class MULTI_API AZombieShooterCharacter : public AMultiCharacter, public IShooterWeaponHolder
{
	GENERATED_BODY()
	/** AI Noise emitter component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UPawnNoiseEmitterComponent* PawnNoiseEmitter;

protected:

	/** Fire weapon input action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* FireAction;

	/** Switch weapon input action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* SwitchWeaponAction;

	// Input Actions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* TeamMessageAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* AllMessageAction;

	//the action to apply the tag
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* TagAction;

	//the actions to get which tag it is
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* Tag1Action;

	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* Tag2Action;

	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* Tag3Action;

	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* Tag4Action;

	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* Tag5Action;
	
	UFUNCTION()
	void OnTagAction();

	//for each seperate tag -- unless there is a way to combine them that i don't know about lol
	UFUNCTION()
	void OnTag1Action();

	UFUNCTION()
	void OnTag2Action();

	UFUNCTION()
	void OnTag3Action();

	UFUNCTION()
	void OnTag4Action();

	UFUNCTION()
	void OnTag5Action();

	void DisplayNewTag();
	
	int TagIndex = 0;
	
	/** Name of the first person mesh weapon socket */
	UPROPERTY(EditAnywhere, Category ="Weapons")
	FName FirstPersonWeaponSocket = FName("HandGrip_R");
	
	/** Name of the third person mesh weapon socket */
	UPROPERTY(EditAnywhere, Category ="Weapons")
	FName ThirdPersonWeaponSocket = FName("HandGrip_R");

	/** Max distance to use for aim traces */
	UPROPERTY(EditAnywhere, Category ="Aim", meta = (ClampMin = 0, ClampMax = 100000, Units = "cm"))
	float MaxAimDistance = 10000.0f;

	/** Max HP this character can have */
	UPROPERTY(EditAnywhere, Category="Health")
	float MaxHP = 500.0f;

	/** Current HP remaining to this character */
	UPROPERTY(ReplicatedUsing=OnRep_CurrentHP)
	float CurrentHP = 0.0f;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentLife)
	int NumOfLives = 3;

	UPROPERTY(Replicated)
	bool bNotDead = false;
	
	UFUNCTION()
	void OnRep_CurrentHP();

	UFUNCTION()
	void OnRep_CurrentLife();

	UFUNCTION()
	void OnRep_Team();

	void DecreaseNumLives(){ NumOfLives--; }

	/** Called to allow Blueprint code to react to the team being set */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta = (DisplayName = "On Team Set"))
	void BP_OnTeamSet();
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Team)
	EZombieShooterTeam Team;
	
	/** Team ID for this character*/
	UPROPERTY(EditAnywhere, Category="Team")
	uint8 TeamByte = 0;
	

	/** List of weapons picked up by the character */
	TArray<AShooterWeapon*> OwnedWeapons;

	/** Weapon currently equipped and ready to shoot with */
	TObjectPtr<AShooterWeapon> CurrentWeapon;
	
	UPROPERTY(Replicated, BlueprintReadOnly)
    FRotator ReplicatedControlRotation;

	UPROPERTY(EditAnywhere, Category ="Destruction", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float RespawnTime = 5.0f;

	FTimerHandle RespawnTimer;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override;

public:

	/** Bullet count updated delegate */
	FZBulletCountUpdatedDelegate OnBulletCountUpdated;

	/** Damaged delegate */
	FZDamagedDelegate OnDamaged;


	void ShowDeathTimer(float Duration);
	void HideDeathTimer();
	bool bIsDead = false;
	float Timer = 5.0f;

	int GenNumLives() { return NumOfLives; }
public:

	void SetTeam(EZombieShooterTeam NewTeam);
	EZombieShooterTeam GetTeam(){ return Team; }

	void SetHealth(float NewHealth);
	
	/** Constructor */
	AZombieShooterCharacter();

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Gameplay cleanup */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

public:

	/** Handle incoming damage */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

public:

	/** Handles start firing input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStartFiring();

	/** Handles stop firing input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStopFiring();

	/** Handles switch weapon input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoSwitchWeapon();

	UFUNCTION(BlueprintCallable, Category="Input")
	void DoSendAllMessage();
	
	UFUNCTION(Server, Reliable)
	void ServerDoStartFiring();

	UFUNCTION(Server, Reliable)
	void ServerDoStopFiring();

	UFUNCTION(Server, Reliable)
	void ServerDoSwitchWeapon();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnDeath();
public:

	//~Begin IShooterWeaponHolder interface

	/** Attaches a weapon's meshes to the owner */
	virtual void AttachWeaponMeshes(AShooterWeapon* Weapon) override;

	/** Plays the firing montage for the weapon */
	virtual void PlayFiringMontage(UAnimMontage* Montage) override;

	/** Applies weapon recoil to the owner */
	virtual void AddWeaponRecoil(float Recoil) override;

	/** Updates the weapon's HUD with the current ammo count */
	virtual void UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize) override;

	/** Calculates and returns the aim location for the weapon */
	virtual FVector GetWeaponTargetLocation() override;

	/** Gives a weapon of this class to the owner */
	virtual void AddWeaponClass(const TSubclassOf<AShooterWeapon>& WeaponClass) override;

	/** Activates the passed weapon */
	virtual void OnWeaponActivated(AShooterWeapon* Weapon) override;

	/** Deactivates the passed weapon */
	virtual void OnWeaponDeactivated(AShooterWeapon* Weapon) override;

	/** Notifies the owner that the weapon cooldown has expired and it's ready to shoot again */
	virtual void OnSemiWeaponRefire() override;

	AShooterWeapon* GetCurrentWeapon(){ return CurrentWeapon;}

	//~End IShooterWeaponHolder interface

protected:
	/** Returns true if the character already owns a weapon of the given class */
	AShooterWeapon* FindWeaponOfType(TSubclassOf<AShooterWeapon> WeaponClass) const;

	/** Called when this character's HP is depleted */
	void Die();

	/** Called to allow Blueprint code to react to this character's death */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta = (DisplayName = "On Death"))
	void BP_OnDeath();

	/** Called from the respawn timer to destroy this character and force the PC to respawn */
	void OnRespawn();
};
