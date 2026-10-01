// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterCharacter.h"
#include "ShooterWeapon.h"
#include "EnhancedInputComponent.h"
#include "ShooterBPLibrary.h"
#include "Components/InputComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "ShooterGameMode.h"
#include "ShooterGameState.h"
#include "ShooterPlayerController.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Variant_Zombie/ZombieShooterGameState.h"


AShooterCharacter::AShooterCharacter()
{
	// create the noise emitter component
	PawnNoiseEmitter = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("Pawn Noise Emitter"));

	// configure movement
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 600.0f, 0.0f);
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	// reset HP to max
	CurrentHP = MaxHP;

	// update the HUD
	OnDamaged.Broadcast(1.0f);

	DisplayNewTag();
}

void AShooterCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the respawn timer
	GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);
}

void AShooterCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Always replicate changes to CurrentHP
	DOREPLIFETIME(AShooterCharacter, CurrentHP);

	DOREPLIFETIME(AShooterCharacter, Team);
	
	DOREPLIFETIME(AShooterCharacter, ReplicatedControlRotation);
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// base class handles move, aim and jump inputs
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Firing
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AShooterCharacter::DoStartFiring);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AShooterCharacter::DoStopFiring);

		// Switch weapon
		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &AShooterCharacter::DoSwitchWeapon);

		//TeamMessageAction AllMessageAction
		//Messages
		EnhancedInputComponent->BindAction(TeamMessageAction, ETriggerEvent::Triggered, this, &AShooterCharacter::DoSendTeamMessage);
		EnhancedInputComponent->BindAction(AllMessageAction, ETriggerEvent::Triggered, this, &AShooterCharacter::DoSendAllMessage);

		EnhancedInputComponent->BindAction(TagAction, ETriggerEvent::Triggered, this, &AShooterCharacter::OnTagAction);

		//for the separate tags
		EnhancedInputComponent->BindAction(Tag1Action, ETriggerEvent::Triggered, this, &AShooterCharacter::OnTag1Action);
		EnhancedInputComponent->BindAction(Tag2Action, ETriggerEvent::Triggered, this, &AShooterCharacter::OnTag2Action);
		EnhancedInputComponent->BindAction(Tag3Action, ETriggerEvent::Triggered, this, &AShooterCharacter::OnTag3Action);
		EnhancedInputComponent->BindAction(Tag4Action, ETriggerEvent::Triggered, this, &AShooterCharacter::OnTag4Action);
		EnhancedInputComponent->BindAction(Tag5Action, ETriggerEvent::Triggered, this, &AShooterCharacter::OnTag5Action);
	}

}

void AShooterCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetLocalRole() == ROLE_Authority)
	{
		ReplicatedControlRotation = GetControlRotation();
	}

	
	if (bIsDead)
	{
		Timer -= DeltaSeconds;

		if (Timer <= 0.0f)
		{
			ShowDeathTimer(0.0f);
			HideDeathTimer();
			
		}
		else
		{
			ShowDeathTimer(Timer);
		}
		
	}
	
}

void AShooterCharacter::OnTagAction()
{
	if (AShooterPlayerController* Cont = GetController<AShooterPlayerController>())
	{
		Cont->ServerApplyTag(TagIndex);
	}
	
}

void AShooterCharacter::OnTag1Action()
{
	TagIndex = 0;

	DisplayNewTag();
}

void AShooterCharacter::OnTag2Action()
{
	TagIndex = 1;

	DisplayNewTag();
}

void AShooterCharacter::OnTag3Action()
{
	TagIndex = 2;

	DisplayNewTag();
}

void AShooterCharacter::OnTag4Action()
{
	TagIndex = 3;
	
	DisplayNewTag();
}

void AShooterCharacter::OnTag5Action()
{
	TagIndex = 4;

	DisplayNewTag();
}

void AShooterCharacter::DisplayNewTag()
{
	if(AShooterPlayerController* Cont = GetController<AShooterPlayerController>())
	{
		Cont->DisplayNewTag(TagIndex);
	}
}

void AShooterCharacter::OnRep_CurrentHP()
{
	OnDamaged.Broadcast(FMath::Max(0.0f, CurrentHP / MaxHP));
}

void AShooterCharacter::OnRep_Team()
{
	BP_OnTeamSet();
}


void AShooterCharacter::ShowDeathTimer(float Duration)
{
	//AShooterPlayerController* SPController = UShooterBPLibrary::GetShooterController(this, 0); Cast<AShooterPlayerController>(Controller)
	if (AShooterPlayerController* SPController = Cast<AShooterPlayerController>(Controller))
	{
		if (SPController->IsLocalPlayerController())
		{
			SPController->ShowDeathTimer(Duration);
		}
	}

	bIsDead = true;
}

void AShooterCharacter::HideDeathTimer()
{
	if (AShooterPlayerController* SPController = Cast<AShooterPlayerController>(Controller))
	{
		if (SPController->IsLocalController())
		{
			SPController->HideDeathTimer();
		}
	}
	
	bIsDead = false;
	Timer = 5.0f;
}

void AShooterCharacter::SetTeam(EShooterTeam InTeam)
{
	Team = InTeam;
    
	if (GetNetMode() == NM_ListenServer)
	{
		OnRep_Team();
	}
}

void AShooterCharacter::SetHealth(float Damage)
{
	CurrentHP -= Damage;
	if (GetNetMode() == NM_ListenServer)
	{
		OnRep_CurrentHP();
	}
}

float AShooterCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// ignore if already dead
	if (CurrentHP <= 0.0f)
	{
		return 0.0f;
	}

	// Reduce HP
	SetHealth(Damage);

	// Have we depleted HP?
	if (CurrentHP <= 0.0f)
	{
		//Increase the GameState score
		if (AShooterGameState* GameState = UShooterBPLibrary::GetShooterGameState(this))
		{
			//Increase the PlayerState score
            AShooterPlayerState* State = EventInstigator->GetPlayerState<AShooterPlayerState>();
            State->SetScore(State->GetScore()+1);
            State->IncrementStreakProgress();
            		
			if (GameState->GetMatchState() == MatchState::InProgress)
			{
				if (State->Team == EShooterTeam::Red)
				{
					GameState->IncreaseRedScore();
				}
				else
				{
					GameState->IncreaseBlueScore();	
				}
				GameState->MulticastStreakMessage(State, GetPlayerState<AShooterPlayerState>());
			}
		}
		
		Die();
		
	}

	// update the HUD
	OnDamaged.Broadcast(FMath::Max(0.0f, CurrentHP / MaxHP));

	return Damage;
}

void AShooterCharacter::DoStartFiring()
{
	ServerDoStartFiring();
}

void AShooterCharacter::DoStopFiring()
{
	ServerDoStopFiring();
}

void AShooterCharacter::DoSwitchWeapon()
{
	ServerDoSwitchWeapon();
}

void AShooterCharacter::DoSendTeamMessage()
{
	if (AShooterPlayerController* ASC = Cast<AShooterPlayerController>(Controller))
	{
		if (ASC->IsLocalController())
		{
			UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(ASC);
			ASC->SendTeamMessage();	
		}
	}
}

void AShooterCharacter::DoSendAllMessage()
{
	if (AShooterPlayerController* ASC = Cast<AShooterPlayerController>(Controller))
	{
		if (ASC->IsLocalController())
		{
			UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(ASC);
			ASC->SendAllMessage();
		}
	}
}

void AShooterCharacter::ServerDoStartFiring_Implementation()
{
	// fire the current weapon
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFiring();
	}
}

void AShooterCharacter::ServerDoStopFiring_Implementation()
{
	// fire the current weapon
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFiring();
	}
}

void AShooterCharacter::ServerDoSwitchWeapon_Implementation()
{
	// ensure we have at least two weapons two switch between
	if (OwnedWeapons.Num() > 1)
	{
		// deactivate the old weapon
		CurrentWeapon->DeactivateWeapon();

		// find the index of the current weapon in the owned list
		int32 WeaponIndex = OwnedWeapons.Find(CurrentWeapon);

		// is this the last weapon?
		if (WeaponIndex == OwnedWeapons.Num() - 1)
		{
			// loop back to the beginning of the array
			WeaponIndex = 0;
		}
		else {
			// select the next weapon index
			++WeaponIndex;
		}

		// set the new weapon as current
		CurrentWeapon = OwnedWeapons[WeaponIndex];

		// activate the new weapon
		CurrentWeapon->ActivateWeapon();
	}
}

void AShooterCharacter::MulticastOnDeath_Implementation()
{
	// reset the bullet counter UI
	OnBulletCountUpdated.Broadcast(0, 0);

	ShowDeathTimer(5.0f);
	
	// call the BP handler
	BP_OnDeath();

	
}


void AShooterCharacter::AttachWeaponMeshes(AShooterWeapon* Weapon)
{
	const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, false);

	// attach the weapon actor
	Weapon->AttachToActor(this, AttachmentRule);

	// attach the weapon meshes
	Weapon->GetFirstPersonMesh()->AttachToComponent(GetFirstPersonMesh(), AttachmentRule, FirstPersonWeaponSocket);
	Weapon->GetThirdPersonMesh()->AttachToComponent(GetMesh(), AttachmentRule, FirstPersonWeaponSocket);
	
}

void AShooterCharacter::PlayFiringMontage(UAnimMontage* Montage) //check for this
{
	
}

void AShooterCharacter::AddWeaponRecoil(float Recoil)
{
	// apply the recoil as pitch input
	AddControllerPitchInput(Recoil);
}

void AShooterCharacter::UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize)
{
	OnBulletCountUpdated.Broadcast(MagazineSize, CurrentAmmo);
}

FVector AShooterCharacter::GetWeaponTargetLocation()
{
	// trace ahead from the camera viewpoint
	FHitResult OutHit;

	const FVector Start = GetFirstPersonCameraComponent()->GetComponentLocation();
	const FVector End = Start + (GetFirstPersonCameraComponent()->GetForwardVector() * MaxAimDistance);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, QueryParams);

	// return either the impact point or the trace end
	return OutHit.bBlockingHit ? OutHit.ImpactPoint : OutHit.TraceEnd;
}

void AShooterCharacter::AddWeaponClass(const TSubclassOf<AShooterWeapon>& WeaponClass)
{
	// do we already own this weapon?
	AShooterWeapon* OwnedWeapon = FindWeaponOfType(WeaponClass);

	if (!OwnedWeapon)
	{
		// spawn the new weapon
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;

		AShooterWeapon* AddedWeapon = GetWorld()->SpawnActor<AShooterWeapon>(WeaponClass, GetActorTransform(), SpawnParams);

		if (AddedWeapon)
		{
			// add the weapon to the owned list
			OwnedWeapons.Add(AddedWeapon);

			// if we have an existing weapon, deactivate it
			if (CurrentWeapon)
			{
				CurrentWeapon->DeactivateWeapon();
			}

			// switch to the new weapon
			CurrentWeapon = AddedWeapon;
			CurrentWeapon->ActivateWeapon();
		}
	}
}

void AShooterCharacter::OnWeaponActivated(AShooterWeapon* Weapon)
{
	// update the bullet counter
	OnBulletCountUpdated.Broadcast(Weapon->GetMagazineSize(), Weapon->GetBulletCount());

	// set the character mesh AnimInstances
	GetFirstPersonMesh()->SetAnimInstanceClass(Weapon->GetFirstPersonAnimInstanceClass());
	GetMesh()->SetAnimInstanceClass(Weapon->GetThirdPersonAnimInstanceClass());
}

void AShooterCharacter::OnWeaponDeactivated(AShooterWeapon* Weapon)
{
	// unused
}

void AShooterCharacter::OnSemiWeaponRefire()
{
	// unused
}

AShooterWeapon* AShooterCharacter::FindWeaponOfType(TSubclassOf<AShooterWeapon> WeaponClass) const
{
	// check each owned weapon
	for (AShooterWeapon* Weapon : OwnedWeapons)
	{
		if (Weapon->IsA(WeaponClass))
		{
			return Weapon;
		}
	}

	// weapon not found
	return nullptr;

}

void AShooterCharacter::Die()
{
	// deactivate the weapon
	if (IsValid(CurrentWeapon))
	{
		CurrentWeapon->DeactivateWeapon();
	}

	// increment the team score
	if (AShooterGameMode* GM = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->IncrementTeamScore(TeamByte);
	}
		
	// stop character movement
	GetCharacterMovement()->StopMovementImmediately();

	// disable controls
	DisableInput(nullptr);

	MulticastOnDeath();

	// schedule character respawn
	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &AShooterCharacter::OnRespawn, RespawnTime, false);
}

void AShooterCharacter::OnRespawn()
{
	
	if (Controller)
	{
		AController* OldController = Controller;
		// Tell the current controller it doesn't control this pawn anymore
		Controller->UnPossess();
		if (AGameModeBase* GameMode = UGameplayStatics::GetGameMode(this))
		{
			// Force the old controller to restart
			GameMode->RestartPlayer(OldController);
		}
	}
	
	// destroy the character to force the PC to respawn
	Destroy();
}
