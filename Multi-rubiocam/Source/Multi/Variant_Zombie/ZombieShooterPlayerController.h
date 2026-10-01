// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)

#pragma once

#include "CoreMinimal.h"
#include "ZombieShooterPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "ZombieShooterPlayerController.generated.h"

class UInputMappingContext;
class AZombieShooterCharacter;
class UShooterBulletCounterUI;
/**
 * 
 */
UCLASS()
class MULTI_API AZombieShooterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input mapping contexts for this player */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** Character class to respawn when the possessed pawn is destroyed */
	UPROPERTY(EditAnywhere, Category="Shooter|Respawn")
	TSubclassOf<class AZombieShooterCharacter> CharacterClass;

	/** Type of bullet counter UI widget to spawn */
	UPROPERTY(EditAnywhere, Category="Shooter|UI")
	TSubclassOf<UShooterBulletCounterUI> BulletCounterUIClass;

	/** Tag to grant the possessed pawn to flag it as the player */
	UPROPERTY(EditAnywhere, Category="Shooter|Player")
	FName PlayerPawnTag = FName("Player");

	/** Pointer to the bullet counter UI widget */
	TObjectPtr<class UShooterBulletCounterUI> BulletCounterUI;

protected:

	/** Gameplay Initialization */
	virtual void BeginPlay() override;

	/** Initialize input bindings */
	virtual void SetupInputComponent() override;

	/** Pawn initialization */
	virtual void OnPossess(APawn* InPawn) override;

	/** Called if the possessed pawn is destroyed */
	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedActor);

	/** Called when the bullet count on the possessed pawn is updated */
	UFUNCTION()
	void OnBulletCountUpdated(int32 MagazineSize, int32 Bullets);

	/** Called when the possessed pawn is damaged */
	UFUNCTION()
	void OnPawnDamaged(float LifePercent);

	virtual void OnRep_Pawn() override;
	void SetupDelegates();
public:
	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<class ATagActor>> TagActorClass;
	
	UFUNCTION(Server, Reliable)
	void ServerApplyTag(int TagIndex);

	//TObjectPtr<class UMultiSaveSystem> SaveSystem;
	void ShowSaveIndicator();
	
	void DisplayNewTag(int Index);
	
	void SendAlert(const FString& DisplayText, FLinearColor Color, float Duration);
	
	void ShowDeathTimer(float TimeToDisplay);
	void HideDeathTimer();

	void SendAllMessage();

	void SendServerMessage(EZombieShooterTeam MessageTeam, const FString& Sender, const FString& Message);

	UFUNCTION(Server, Reliable)
	void ClientOnPossess();

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	UPROPERTY(EditAnywhere, Category="Projectile|Hit")
	TSubclassOf<UDamageType> ZombieDamageType;
	
};
