// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/ShooterPlayerController.h"

#include "EngineUtils.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "ShooterCharacter.h"
#include "ShooterBulletCounterUI.h"
#include "Multi.h"
#include "MultiSaveGame.h"
#include "MultiSaveSystem.h"
#include "ShooterBPLibrary.h"
#include "ShooterPlayerState.h"
#include "TagActor.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Widgets/Input/SVirtualJoystick.h"

void AShooterPlayerController::BeginPlay()
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

void AShooterPlayerController::SetupInputComponent()
{
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

void AShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AShooterPlayerState* State = InPawn->GetPlayerState<AShooterPlayerState>();
	
	if (AShooterCharacter* Char = Cast<AShooterCharacter>(InPawn))
	{
		Char->SetTeam(State->Team);
	}

	if (GetNetMode() == NM_ListenServer)
	{
		SetupDelegates();
	}
}

void AShooterPlayerController::ClientOnPossess_Implementation()
{
	UWidgetBlueprintLibrary::SetInputMode_GameOnly(this);
}

void AShooterPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	// reset the bullet counter HUD
	BulletCounterUI->BP_UpdateBulletCounter(0, 0);
}

void AShooterPlayerController::OnBulletCountUpdated(int32 MagazineSize, int32 Bullets)
{
	// update the UI
	if (BulletCounterUI)
	{
		BulletCounterUI->BP_UpdateBulletCounter(MagazineSize, Bullets);
	}
}

void AShooterPlayerController::OnPawnDamaged(float LifePercent)
{
	if (IsValid(BulletCounterUI))
	{
		BulletCounterUI->BP_Damaged(LifePercent);
	}
}

void AShooterPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	if (GetNetMode() == NM_ListenServer)
	{
		SetupDelegates();
	}
}

void AShooterPlayerController::SetupDelegates()
{
	// Only mess with the delegates on the local controller (since they're for the UI)
	if (GetPawn() && IsLocalPlayerController())
	{
		// subscribe to the pawn's OnDestroyed delegate
		GetPawn()->OnDestroyed.AddUniqueDynamic(this, &AShooterPlayerController::OnPawnDestroyed);

		// is this a shooter character?
		if (AShooterCharacter* ShooterCharacter = GetPawn<AShooterCharacter>())
		{
			ShooterCharacter->OnBulletCountUpdated.AddUniqueDynamic(this, &AShooterPlayerController::OnBulletCountUpdated);
			ShooterCharacter->OnDamaged.AddUniqueDynamic(this, &AShooterPlayerController::OnPawnDamaged);

			// force update the life bar
			ShooterCharacter->OnDamaged.Broadcast(1.0f);
		}
	}
}

void AShooterPlayerController::ServerApplyTag_Implementation(int TagIndex)
{
	if (AShooterCharacter* PlayerChar = GetPawn<AShooterCharacter>())
	{
		// We need to ignore all the shooter characters in the world
		FCollisionQueryParams CollisionParams;
		for (TActorIterator<AShooterCharacter> Iter(GetWorld()); Iter; ++Iter)
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

void AShooterPlayerController::ShowSaveIndicator()
{
	BulletCounterUI->ShowSaveIndicator();
}

void AShooterPlayerController::DisplayNewTag(int Index)
{
	BulletCounterUI->ChangeDisplayMaterial(Index);
}

void AShooterPlayerController::SendAlert(const FString& DisplayText, FLinearColor Color, float Duration)
{
	BulletCounterUI->ShowAlert(DisplayText, Color, Duration);
}

void AShooterPlayerController::SendWinnerMessage(FLinearColor Winner)
{
	BulletCounterUI->ShowWinnerMessage(Winner);
}

void AShooterPlayerController::HideWinner()
{
	BulletCounterUI->HideWinnerMessage();
}

void AShooterPlayerController::SendStreakMessage(class AShooterPlayerState* Killer, class AShooterPlayerState* Killed)
{
	BulletCounterUI->ShowStreakMessage(Killer, Killed);
}

void AShooterPlayerController::HideStreakMessage()
{
	BulletCounterUI->HideStreakMessage();
}

void AShooterPlayerController::ShowDeathTimer(float Duration)
{
	BulletCounterUI->ShowDeathTimer(Duration);
}

void AShooterPlayerController::HideDeathTimer()
{
	BulletCounterUI->HideDeathTimer();
}

void AShooterPlayerController::SendTeamMessage()
{
	UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(this, Cast<UWidget>(BulletCounterUI));
	if (AShooterPlayerState* PS = GetPlayerState<AShooterPlayerState>())
	{
		BulletCounterUI->SendMessage(PS->Team);		
	}
}

void AShooterPlayerController::SendAllMessage()
{
	UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(this, Cast<UWidget>(BulletCounterUI));
	BulletCounterUI->SendMessage(EShooterTeam::None);
}

void AShooterPlayerController::SendServerMessage(EShooterTeam MessageTeam, const FString& Sender,
	const FString& Message)
{
	BulletCounterUI->AddChatMessage(MessageTeam,  Sender, Message);
}


