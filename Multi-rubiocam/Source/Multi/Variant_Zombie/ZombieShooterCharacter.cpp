// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)


#include "ZombieShooterCharacter.h"

#include "EnhancedInputComponent.h"
#include "ShooterWeapon.h"
#include "ZombieShooterBPLibrary.h"
#include "ZombieShooterGameState.h"
#include "ZombieShooterPlayerController.h"
#include "ZombieShooterPlayerState.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


AZombieShooterCharacter::AZombieShooterCharacter()
{
	// create the noise emitter component
	PawnNoiseEmitter = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("Pawn Noise Emitter"));

	// configure movement
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 600.0f, 0.0f);
}

void AZombieShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	// reset HP to max
	CurrentHP = MaxHP;

	// update the HUD
	OnDamaged.Broadcast(1.0f);

	DisplayNewTag();
}

void AZombieShooterCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the respawn timer
	GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);
}

void AZombieShooterCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Always replicate changes to CurrentHP
	DOREPLIFETIME(AZombieShooterCharacter, CurrentHP);
	DOREPLIFETIME(AZombieShooterCharacter, Team);
	DOREPLIFETIME(AZombieShooterCharacter, ReplicatedControlRotation);
	DOREPLIFETIME(AZombieShooterCharacter, NumOfLives);
}

void AZombieShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (Cast<AZombieShooterPlayerController>(GetController()) == UZombieShooterBPLibrary::GetZombieShooterController(this, 0))
	{
		// Set up action bindings
		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
		{
			// Firing
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AZombieShooterCharacter::DoStartFiring);
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AZombieShooterCharacter::DoStopFiring);

			// Switch weapon
			EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &AZombieShooterCharacter::DoSwitchWeapon);

			//Messages
			EnhancedInputComponent->BindAction(AllMessageAction, ETriggerEvent::Triggered, this, &AZombieShooterCharacter::DoSendAllMessage);

			EnhancedInputComponent->BindAction(TagAction, ETriggerEvent::Triggered, this, &AZombieShooterCharacter::OnTagAction);

			//for the separate tags
			EnhancedInputComponent->BindAction(Tag1Action, ETriggerEvent::Triggered, this, &AZombieShooterCharacter::OnTag1Action);
			EnhancedInputComponent->BindAction(Tag2Action, ETriggerEvent::Triggered, this, &AZombieShooterCharacter::OnTag2Action);
			EnhancedInputComponent->BindAction(Tag3Action, ETriggerEvent::Triggered, this, &AZombieShooterCharacter::OnTag3Action);
			EnhancedInputComponent->BindAction(Tag4Action, ETriggerEvent::Triggered, this, &AZombieShooterCharacter::OnTag4Action);
			EnhancedInputComponent->BindAction(Tag5Action, ETriggerEvent::Triggered, this, &AZombieShooterCharacter::OnTag5Action);
		}
	}
}

void AZombieShooterCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (Cast<AZombieShooterPlayerController>(GetController()) == UZombieShooterBPLibrary::GetZombieShooterController(this, 0))
	{
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

	
	if (NumOfLives <= 0 && !bNotDead)
	{
		bNotDead = true;
		AZombieShooterGameState* GS = UZombieShooterBPLibrary::GetZombieShooterGameState(this);
		GS->IncreaseNumDead();
		if (GetLocalRole() == ROLE_Authority)
		{
			UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx( UZombieShooterBPLibrary::GetZombieShooterController(this, 0));
		}
	}
}

void AZombieShooterCharacter::OnTagAction()
{
	if (AZombieShooterPlayerController* Cont = GetController<AZombieShooterPlayerController>())
	{
		Cont->ServerApplyTag(TagIndex);
	}
}

void AZombieShooterCharacter::OnTag1Action()
{
	TagIndex = 0;
	DisplayNewTag();
}

void AZombieShooterCharacter::OnTag2Action()
{
	TagIndex = 1;
	DisplayNewTag();
}

void AZombieShooterCharacter::OnTag3Action()
{
	TagIndex = 2;
	DisplayNewTag();
}

void AZombieShooterCharacter::OnTag4Action()
{
	TagIndex = 3;
	DisplayNewTag();
}

void AZombieShooterCharacter::OnTag5Action()
{
	TagIndex = 4;
	DisplayNewTag();
}

void AZombieShooterCharacter::DisplayNewTag()
{
	if(AZombieShooterPlayerController* Cont = GetController<AZombieShooterPlayerController>())
	{
		Cont->DisplayNewTag(TagIndex);
	}
}

void AZombieShooterCharacter::OnRep_CurrentHP()
{
	OnDamaged.Broadcast(FMath::Max(0.0f, CurrentHP / MaxHP));
}

void AZombieShooterCharacter::OnRep_CurrentLife()
{
	AZombieShooterPlayerState* State = GetPlayerState<AZombieShooterPlayerState>();
	NumOfLives = State->GetScore()-1;
}

void AZombieShooterCharacter::OnRep_Team()
{
	BP_OnTeamSet();
}

void AZombieShooterCharacter::ShowDeathTimer(float Duration)
{
	if (AZombieShooterPlayerController* SPController = Cast<AZombieShooterPlayerController>(Controller))
	{
		if (SPController->IsLocalPlayerController())
		{
			SPController->ShowDeathTimer(Duration);
		}

		bIsDead = true;
	}

}

void AZombieShooterCharacter::HideDeathTimer()
{
	if (AZombieShooterPlayerController* SPController = Cast<AZombieShooterPlayerController>(Controller))
	{
		if (SPController->IsLocalController())
		{
			SPController->HideDeathTimer();
		}
		bIsDead = false;
		Timer = 5.0f;
	}
}

void AZombieShooterCharacter::SetTeam(EZombieShooterTeam NewTeam)
{
	if (AZombieShooterPlayerController* SPController = Cast<AZombieShooterPlayerController>(Controller))
	{
		Team = NewTeam;
		
		if (GetNetMode() == NM_ListenServer)
		{
			OnRep_Team();
		}
	}
}

void AZombieShooterCharacter::SetHealth(float Damage)
{
	CurrentHP -= Damage;
	if (GetNetMode() == NM_ListenServer)
	{
		OnRep_CurrentHP();
	}
}

float AZombieShooterCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
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
		Die();
	}

	// update the HUD
	OnDamaged.Broadcast(FMath::Max(0.0f, CurrentHP / MaxHP));

	return Damage;
}

void AZombieShooterCharacter::DoStartFiring()
{
	ServerDoStartFiring();
}

void AZombieShooterCharacter::DoStopFiring()
{
	ServerDoStopFiring();
}

void AZombieShooterCharacter::DoSwitchWeapon()
{
	ServerDoSwitchWeapon();
}

void AZombieShooterCharacter::DoSendAllMessage()
{
	if (AZombieShooterPlayerController* ASC = Cast<AZombieShooterPlayerController>(Controller))
	{
		if (ASC->IsLocalController())
		{
			UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(ASC);
			ASC->SendAllMessage();
		}
	}
}

void AZombieShooterCharacter::ServerDoStartFiring_Implementation()
{
	// fire the current weapon
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFiring();
	}
}

void AZombieShooterCharacter::ServerDoStopFiring_Implementation()
{
	// fire the current weapon
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFiring();
	}
}

void AZombieShooterCharacter::ServerDoSwitchWeapon_Implementation()
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

void AZombieShooterCharacter::MulticastOnDeath_Implementation()
{
	if (AZombieShooterPlayerController* SPController = Cast<AZombieShooterPlayerController>(Controller))
	{
		// reset the bullet counter UI
		OnBulletCountUpdated.Broadcast(0, 0);

		ShowDeathTimer(5.0f);

		AZombieShooterPlayerState* State = GetPlayerState<AZombieShooterPlayerState>();
		State->SetScore(State->GetScore()-1);
	}
	
	// call the BP handler
	BP_OnDeath();

}

void AZombieShooterCharacter::AttachWeaponMeshes(AShooterWeapon* Weapon)
{
	const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, false);

	// attach the weapon actor
	Weapon->AttachToActor(this, AttachmentRule);

	// attach the weapon meshes
	Weapon->GetFirstPersonMesh()->AttachToComponent(GetFirstPersonMesh(), AttachmentRule, FirstPersonWeaponSocket);
	Weapon->GetThirdPersonMesh()->AttachToComponent(GetMesh(), AttachmentRule, FirstPersonWeaponSocket);
}


void AZombieShooterCharacter::PlayFiringMontage(UAnimMontage* Montage)
{
}

void AZombieShooterCharacter::AddWeaponRecoil(float Recoil)
{
	// apply the recoil as pitch input
	AddControllerPitchInput(Recoil);
}

void AZombieShooterCharacter::UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize)
{
	OnBulletCountUpdated.Broadcast(MagazineSize, CurrentAmmo);
}

FVector AZombieShooterCharacter::GetWeaponTargetLocation()
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

void AZombieShooterCharacter::AddWeaponClass(const TSubclassOf<AShooterWeapon>& WeaponClass)
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

void AZombieShooterCharacter::OnWeaponActivated(AShooterWeapon* Weapon)
{
	// update the bullet counter
	OnBulletCountUpdated.Broadcast(Weapon->GetMagazineSize(), Weapon->GetBulletCount());

	// set the character mesh AnimInstances
	GetFirstPersonMesh()->SetAnimInstanceClass(Weapon->GetFirstPersonAnimInstanceClass());
	GetMesh()->SetAnimInstanceClass(Weapon->GetThirdPersonAnimInstanceClass());
}

void AZombieShooterCharacter::OnWeaponDeactivated(AShooterWeapon* Weapon)
{
}

void AZombieShooterCharacter::OnSemiWeaponRefire()
{
}

AShooterWeapon* AZombieShooterCharacter::FindWeaponOfType(TSubclassOf<AShooterWeapon> WeaponClass) const
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

void AZombieShooterCharacter::Die()
{
	// deactivate the weapon
	if (IsValid(CurrentWeapon))
	{
		CurrentWeapon->DeactivateWeapon();
	}
	
	// stop character movement
	GetCharacterMovement()->StopMovementImmediately();

	// disable controls
	DisableInput(nullptr);

	MulticastOnDeath();
	
	if (AZombieShooterPlayerController* SPController = Cast<AZombieShooterPlayerController>(Controller))
	{
		// schedule character respawn
		GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &AZombieShooterCharacter::OnRespawn, RespawnTime, false);
		
		if (GetNetMode() == NM_ListenServer)
		{
			OnRep_CurrentLife();
		}
		else
		{
			NumOfLives--;
		}
	}
}

void AZombieShooterCharacter::OnRespawn()
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
