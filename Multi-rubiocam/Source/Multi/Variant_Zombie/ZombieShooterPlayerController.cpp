// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)


#include "ZombieShooterPlayerController.h"

#include "EngineUtils.h"
#include "EnhancedInputSubsystems.h"
#include "Multi.h"
#include "MultiSaveGame.h"
#include "MultiSaveSystem.h"
#include "ShooterBulletCounterUI.h"
#include "TagActor.h"
#include "ZombieShooterCharacter.h"
#include "ZombieShooterPlayerState.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Input/SVirtualJoystick.h"

void AZombieShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController())
	{
		if (SVirtualJoystick::ShouldDisplayTouchInterface())
		{
			// spawn the mobile controls widget
			MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

			if (MobileControlsWidget)
			{
				// add the controls to the player screen
				MobileControlsWidget->AddToPlayerScreen(0);

			} else {

				
				UE_LOG(LogMulti, Error, TEXT("Could not spawn mobile controls widget."));

			}
		}

		// create the bullet counter widget and add it to the screen
		BulletCounterUI = CreateWidget<UShooterBulletCounterUI>(this, BulletCounterUIClass);

		if (BulletCounterUI)
		{
			BulletCounterUI->AddToPlayerScreen(0);
			BulletCounterUI->SetOwningPlayer(this);
			UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(this, BulletCounterUI);

		} else {

			UE_LOG(LogMulti, Error, TEXT("Could not spawn bullet counter widget."));

		}
		
	}
	
}

void AZombieShooterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// add the input mapping contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			//i fear i gotta keep this or I can't turn/adjust my view			
			// only add these IMCs if we're not using mobile touch input
			if (!SVirtualJoystick::ShouldDisplayTouchInterface())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

void AZombieShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AZombieShooterPlayerState* State = InPawn->GetPlayerState<AZombieShooterPlayerState>();
	
	if (AZombieShooterCharacter* Char = Cast<AZombieShooterCharacter>(InPawn))
	{
		Char->SetTeam(State->Team);
	}

	if (GetNetMode() == NM_ListenServer)
	{
		SetupDelegates();
	}
}

void AZombieShooterPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	// reset the bullet counter HUD
	BulletCounterUI->BP_UpdateBulletCounter(0, 0);
}

void AZombieShooterPlayerController::OnBulletCountUpdated(int32 MagazineSize, int32 Bullets)
{
	// update the UI
	if (BulletCounterUI)
	{
		BulletCounterUI->BP_UpdateBulletCounter(MagazineSize, Bullets);
	}
}

void AZombieShooterPlayerController::OnPawnDamaged(float LifePercent)
{
	if (IsValid(BulletCounterUI))
	{
		BulletCounterUI->BP_Damaged(LifePercent);
	}
}

void AZombieShooterPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	if (GetNetMode() == NM_ListenServer)
	{
		SetupDelegates();
	}
}

void AZombieShooterPlayerController::SetupDelegates()
{
	// Only mess with the delegates on the local controller (since they're for the UI)
	if (GetPawn() && IsLocalPlayerController())
	{
		// subscribe to the pawn's OnDestroyed delegate
		GetPawn()->OnDestroyed.AddUniqueDynamic(this, &AZombieShooterPlayerController::OnPawnDestroyed);

		// is this a shooter character?
		if (AZombieShooterCharacter* ShooterCharacter = GetPawn<AZombieShooterCharacter>())
		{
			ShooterCharacter->OnBulletCountUpdated.AddUniqueDynamic(this, &AZombieShooterPlayerController::OnBulletCountUpdated);
			ShooterCharacter->OnDamaged.AddUniqueDynamic(this, &AZombieShooterPlayerController::OnPawnDamaged);

			// force update the life bar
			ShooterCharacter->OnDamaged.Broadcast(1.0f);
		}
	}
}

void AZombieShooterPlayerController::ServerApplyTag_Implementation(int TagIndex)
{
	
	if (AZombieShooterCharacter* PlayerChar = GetPawn<AZombieShooterCharacter>())
	{
		// We need to ignore all the shooter characters in the world
		FCollisionQueryParams CollisionParams;
		for (TActorIterator<AZombieShooterCharacter> Iter(GetWorld()); Iter; ++Iter)
		{
			CollisionParams.AddIgnoredActor(*Iter);
		}

		// Do a line trace 500 units (5m) in the direction we're facing
		FHitResult HitResult;
		FVector StartPoint = PlayerChar->GetActorLocation();
		FVector EndPoint = StartPoint + ControlRotation.Vector() * 500.0f;
		GetWorld()->LineTraceSingleByChannel(HitResult, StartPoint, EndPoint, ECC_Camera, CollisionParams);

		if (HitResult.bBlockingHit)
		{
			// Setup spawn parameters to always spawn the tag actor, even if colliding
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			// Spawn at the hit location with specified rotation
			FRotator SpawnRotation = FRotator::ZeroRotator;
			ATagActor* TagActor = GetWorld()->SpawnActor<ATagActor>(TagActorClass[TagIndex], HitResult.Location, SpawnRotation, SpawnParameters);
			if (TagActor)
			{
				FQuat Quaternion;
				
				float Dot = FVector::DotProduct(FVector::XAxisVector, -HitResult.Normal);
				// Handle collinear case where initial and desired facing are the same (no rotation)
				if (Dot == 1.0f)
				{
					Quaternion = FQuat::Identity;
				}
				// Handle collinear case where we have to yaw 180 degrees
				else if (Dot == -1.0f)
				{
					Quaternion = FQuat(FVector::ZAxisVector, UE_PI);
				}
				else
				{
					// Axis of rotation is initial facing (unit x) cross desired facing
					FVector Axis = FVector::CrossProduct(FVector::XAxisVector, -HitResult.Normal);
					Axis.Normalize();
					float Angle = FMath::Acos(Dot);
					Quaternion = FQuat(Axis, Angle);
				}
				
				//convert the quaternion to a rotator
				SpawnRotation = FRotator(Quaternion);
				// If we're on the ground/ceiling, yaw based on player control yaw
				// Use nearly zero with an error tolerance of 0.1 (default tolerance is too strict)
				if (!FMath::IsNearlyZero(SpawnRotation.Pitch, 0.1f))
				{
					SpawnRotation.Yaw = ControlRotation.Yaw + 90.0f;
				}
				// Otherwise, apply the 90 degrees of roll
				else
				{
					SpawnRotation.Roll += 90.0f;
				}

				TagActor->SetActorRotation(SpawnRotation);

				FTagActorSaveData ToAdd;
				ToAdd.ClassType = TagActorClass[TagIndex];
				ToAdd.Transform = TagActor->GetTransform();

				UMultiSaveSystem* MSS = GetWorld()->GetGameInstance()->GetSubsystem<UMultiSaveSystem>();
				MSS->UpdateSave(ToAdd);
			}
		}
	}
}

void AZombieShooterPlayerController::ShowSaveIndicator()
{
	BulletCounterUI->ShowSaveIndicator();
}

void AZombieShooterPlayerController::DisplayNewTag(int Index)
{
	BulletCounterUI->ChangeDisplayMaterial(Index);
}

void AZombieShooterPlayerController::SendAlert(const FString& DisplayText, FLinearColor Color, float Duration)
{
	BulletCounterUI->ShowAlert(DisplayText, Color, Duration);
}

void AZombieShooterPlayerController::ShowDeathTimer(float TimeToDisplay)
{
	BulletCounterUI->ShowDeathTimer(TimeToDisplay);
}

void AZombieShooterPlayerController::HideDeathTimer()
{
	BulletCounterUI->HideDeathTimer();
}

void AZombieShooterPlayerController::SendAllMessage()
{
	UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(this, Cast<UWidget>(BulletCounterUI));
	BulletCounterUI->SendMessageZVer(EZombieShooterTeam::None);
}

void AZombieShooterPlayerController::SendServerMessage(EZombieShooterTeam MessageTeam, const FString& Sender,
	const FString& Message)
{
	BulletCounterUI->AddChatMessageZVer(MessageTeam,  Sender, Message);
}

void AZombieShooterPlayerController::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (AZombieShooterCharacter* ZChar = Cast<AZombieShooterCharacter>(OtherActor))
	{
		UGameplayStatics::ApplyDamage(OtherActor, 5.0f, this, GetOwner(), ZombieDamageType);
	}
	
}

void AZombieShooterPlayerController::ClientOnPossess_Implementation()
{
	UWidgetBlueprintLibrary::SetInputMode_GameOnly(this);
}
