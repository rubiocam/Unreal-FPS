// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterBulletCounterUI.h"

#include "ChatMessageWidget.h"
#include "ShooterBPLibrary.h"
#include "ShooterCharacter.h"
#include "ShooterGameState.h"
#include "ShooterPlayerController.h"
#include "ShooterPlayerState.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/CircularThrobber.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Variant_Zombie/ZombieShooterBPLibrary.h"
#include "Variant_Zombie/ZombieShooterGameState.h"
#include "Variant_Zombie/ZombieShooterPlayerController.h"
#include "Variant_Zombie/ZombieShooterPlayerState.h"

void UShooterBulletCounterUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (AShooterGameState* State = UShooterBPLibrary::GetShooterGameState(this))
	{
		int32 BlueTeamScore = State->BlueTeamScore;
		int32 RedTeamScore = State->RedTeamScore;

		
		RedScore.Get()->SetText(FText::FromString(FString::FromInt(RedTeamScore)));
		BlueScore.Get()->SetText(FText::FromString(FString::FromInt(BlueTeamScore)));
		
		if (AShooterPlayerController* Controller = UShooterBPLibrary::GetShooterController(this, 0))
		{
			if (AShooterPlayerState* PlayerState = Controller->GetPlayerState<AShooterPlayerState>())
			{
				int32 Score = PlayerState->GetScore();
				MyScore.Get()->SetText(FText::FromString(FString::FromInt(Score)));
			}
		}

		
		if (AShooterGameState* GS = UShooterBPLibrary::GetShooterGameState(this))
		{
			// If we aren't in progress, show the timer
			if (GS && !GS->IsMatchInProgress() && GS->GetMatchState() == MatchState::WaitingToStart)
			{
				// If we can't see the timer, make it visible
				if (Timer->GetVisibility() != ESlateVisibility::HitTestInvisible)
				{
					Timer->SetVisibility(ESlateVisibility::HitTestInvisible);
				}
				FString TimerText = FString::Printf(TEXT("%f"), GS->WaitingToStartTime);
				Timer->SetText(FText::FromString(TimerText));
				ReadyCheckBox->SetVisibility(ESlateVisibility::Visible);
			}
			else if (Timer->GetVisibility() != ESlateVisibility::Hidden)
			{
				// Hide this because match is in progress
				Timer->SetVisibility(ESlateVisibility::Hidden);
				ReadyCheckBox->SetVisibility(ESlateVisibility::Hidden);
				UWidgetBlueprintLibrary::SetInputMode_GameOnly(UShooterBPLibrary::GetShooterController(this, 0));
				TagImage->SetVisibility(ESlateVisibility::Visible);
				ChangeDisplayMaterial(0);
			}
		}

		if (StartTimer <= 0)
		{
			StartMatchText->SetVisibility(ESlateVisibility::Hidden);
		}

		StartTimer -= InDeltaTime;

		if (SIOnScreen)
		{
			SITimer -= InDeltaTime;
			if (SITimer < 0)
			{
				SaveIndicator->SetVisibility(ESlateVisibility::Hidden);
				SITimer = 0.25;
				SIOnScreen = false;
			}
		}
	}
	else if (AZombieShooterGameState* ZGS = UZombieShooterBPLibrary::GetZombieShooterGameState(this))
	{

		int32 BlueTeamScore = ZGS->BlueTeamScore;
		int32 RedTeamScore = ZGS->RedTeamScore;

		
		RedScore.Get()->SetText(FText::FromString(FString::FromInt(RedTeamScore)));
		BlueScore.Get()->SetText(FText::FromString(FString::FromInt(BlueTeamScore)));
		
		if (AZombieShooterPlayerController* Cont = UZombieShooterBPLibrary::GetZombieShooterController(this, 0))
		{
			if (AZombieShooterPlayerState* ZPS = Cont->GetPlayerState<AZombieShooterPlayerState>())
			{
				int32 Score = ZPS->GetScore();
				MyScore.Get()->SetText(FText::FromString(FString::FromInt(Score)));
			}
		}
		
		// If we aren't in progress, show the timer
		if (ZGS && !ZGS->IsMatchInProgress() && ZGS->GetMatchState() == MatchState::WaitingToStart){
			// If we can't see the timer, make it visible
			if (Timer->GetVisibility() != ESlateVisibility::HitTestInvisible)
			{
				Timer->SetVisibility(ESlateVisibility::HitTestInvisible);
			}
			FString TimerText = FString::Printf(TEXT("%f"), ZGS->WaitingToStartTime);
			Timer->SetText(FText::FromString(TimerText));
			ReadyCheckBox->SetVisibility(ESlateVisibility::Visible);
		}
		else if (Timer->GetVisibility() != ESlateVisibility::Hidden)
		{
			// Hide this because match is in progress
			Timer->SetVisibility(ESlateVisibility::Hidden);
			ReadyCheckBox->SetVisibility(ESlateVisibility::Hidden);
			UWidgetBlueprintLibrary::SetInputMode_GameOnly(UZombieShooterBPLibrary::GetZombieShooterController(this, 0));
			TagImage->SetVisibility(ESlateVisibility::Visible);
			ChangeDisplayMaterial(0);
		}
		
		if (StartTimer <= 0)
		{
			StartMatchText->SetVisibility(ESlateVisibility::Hidden);
		}
    
		StartTimer -= InDeltaTime;
    
		if (SIOnScreen)
		{
			SITimer -= InDeltaTime;
			if (SITimer < 0)
			{
				SaveIndicator->SetVisibility(ESlateVisibility::Hidden);
				SITimer = 0.25;
				SIOnScreen = false;
			}
		}
	}	
}

void UShooterBulletCounterUI::NativeConstruct()
{
	Super::NativeConstruct();

	ChatTextBox->OnTextCommitted.AddDynamic(this, &UShooterBulletCounterUI::OnEnter);
	ReadyCheckBox->OnCheckStateChanged.AddDynamic(this, &UShooterBulletCounterUI::DoOnCheckStateChanged);
}

void UShooterBulletCounterUI::NativeDestruct()
{
	Super::NativeDestruct();

	ChatTextBox->OnTextCommitted.RemoveAll(this);
}

void UShooterBulletCounterUI::ChangeDisplayMaterial(int Index)
{
	TagImage->SetBrushFromMaterial(TagMaterials[Index]);
}

void UShooterBulletCounterUI::ShowAlert(const FString& DisplayText, FLinearColor Color, float Duration)
{
	StartMatchText->SetText(FText::FromString(DisplayText));
	StartMatchText->SetColorAndOpacity(FSlateColor(Color));
	StartMatchText->SetVisibility(ESlateVisibility::Visible);
	StartTimer = Duration;
}

void UShooterBulletCounterUI::ShowWinnerMessage(FLinearColor Winner)
{
	DisplayWinnerText->SetColorAndOpacity(FSlateColor(Winner));
	DisplayWinnerText->SetVisibility(ESlateVisibility::Visible);

	if (Winner == FLinearColor::Red)
	{
		DisplayWinnerText->SetText(FText::FromString("RED TEAM WINS"));
	}
	else
	{
		DisplayWinnerText->SetText(FText::FromString("BLUE TEAM WINS"));
	}
	
}

void UShooterBulletCounterUI::HideWinnerMessage()
{
	DisplayWinnerText->SetVisibility(ESlateVisibility::Hidden);
}

void UShooterBulletCounterUI::ShowStreakMessage(class AShooterPlayerState* Killer, class AShooterPlayerState* Killed)
{
	EShooterTeam Team = Killer->Team;
	FLinearColor TextColor;

	if (Team == EShooterTeam::Red)
	{
		TextColor = FLinearColor::Red;
	}
	else
	{
		TextColor = FLinearColor::Blue;
	}

	if (Killer->SpreeProgress == 3 || Killer->SpreeProgress == 5 || Killer->SpreeProgress == 7 || Killer->SpreeProgress == 9)
	{
		FString SpreeTextBlock = Killer->GetName();

		if (Killer->SpreeProgress == 3)
		{
			SpreeTextBlock = SpreeTextBlock + " is on a killing spree!";	
		}
		else if (Killer->SpreeProgress == 5)
		{
			SpreeTextBlock =SpreeTextBlock + " is dominating!";
		}
		else if (Killer->SpreeProgress == 7)
		{
			SpreeTextBlock = SpreeTextBlock + " is UNSTOPPABLE!";
		}
		else if (Killer->SpreeProgress == 9)
		{
			SpreeTextBlock = SpreeTextBlock + " is GODLIKE!";
		}
		
		SpreeText->SetColorAndOpacity(FSlateColor(TextColor));
		SpreeText->SetVisibility(ESlateVisibility::Visible);
		SpreeText->SetText(FText::FromString(SpreeTextBlock));
		
		PlaySpreeSound(Killer->Sounds[Killer->SpreeProgress]);
	}
	else if (Killed->SpreeProgress > 3)
	{
		FString SpreeTextBlock = Killer->GetName();
		SpreeTextBlock = SpreeTextBlock + " ended ";
		SpreeTextBlock = SpreeTextBlock + Killed->GetName();
		SpreeTextBlock = SpreeTextBlock + "’s streak!";

		SpreeText->SetColorAndOpacity(FSlateColor(TextColor));
		SpreeText->SetVisibility(ESlateVisibility::Visible);
		SpreeText->SetText(FText::FromString(SpreeTextBlock));
		Killed->ResetStreakProgress();
	}
	
}

void UShooterBulletCounterUI::HideStreakMessage()
{
	SpreeText->SetVisibility(ESlateVisibility::Hidden);
}

void UShooterBulletCounterUI::ShowDeathTimer(float Duration)
{
	FString TimerText = "You are dead :( \n";
	TimerText += FString::Printf(TEXT("%f"), Duration);
	DeathText->SetText(FText::FromString(TimerText));
	DeathText->SetVisibility(ESlateVisibility::Visible);
	
}

void UShooterBulletCounterUI::HideDeathTimer()
{
	DeathText->SetVisibility(ESlateVisibility::Hidden);
}

void UShooterBulletCounterUI::AddChatMessage(EShooterTeam Team, const FString& Sender, const FString& Message)
{
	
	UChatMessageWidget* ChatMessage = NewObject<UChatMessageWidget>(this, ChatMessageWidget);
	ChatMessages->AddChildToVerticalBox(ChatMessage);
	ChatMessage->SetChatMessage(Team, Sender, Message);
	ChatScrollBox->ScrollToEnd();

}

void UShooterBulletCounterUI::SendMessage(EShooterTeam Team)
{
	ChatEntry->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	if (Team == EShooterTeam::Blue)
	{
		ChatTeam->SetText(FText::FromString(TEXT("[BLUE]")));
		ChatTeam->SetColorAndOpacity(FLinearColor::Blue);
	}
	else if (Team == EShooterTeam::Red)
	{
		ChatTeam->SetText(FText::FromString(TEXT("[RED]")));
		ChatTeam->SetColorAndOpacity(FLinearColor::Red);
	}
	else
	{
		ChatTeam->SetText(FText::FromString(TEXT("[ALL]")));
		ChatTeam->SetColorAndOpacity(FLinearColor::White);
	}
	
	ChatTextBox->SetFocus();
}

void UShooterBulletCounterUI::AddChatMessageZVer(EZombieShooterTeam Team, const FString& Sender, const FString& Message)
{
	UChatMessageWidget* ChatMessage = NewObject<UChatMessageWidget>(this, ChatMessageWidget);
	ChatMessages->AddChildToVerticalBox(ChatMessage);
	ChatMessage->SetChatMessageZVer(Team, Sender, Message);
	ChatScrollBox->ScrollToEnd();
	
}

void UShooterBulletCounterUI::SendMessageZVer(EZombieShooterTeam Team)
{
	ChatEntry->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	if (Team == EZombieShooterTeam::Blue)
	{
		ChatTeam->SetText(FText::FromString(TEXT("[BLUE]")));
		ChatTeam->SetColorAndOpacity(FLinearColor::Blue);
	}
	else if (Team == EZombieShooterTeam::Red)
	{
		ChatTeam->SetText(FText::FromString(TEXT("[RED]")));
		ChatTeam->SetColorAndOpacity(FLinearColor::Red);
	}
	else
	{
		ChatTeam->SetText(FText::FromString(TEXT("[ALL]")));
		ChatTeam->SetColorAndOpacity(FLinearColor::White);
	}
	
	ChatTextBox->SetFocus();
}


void UShooterBulletCounterUI::OnEnter(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		if (AShooterPlayerState* PS = GetOwningPlayerState<AShooterPlayerState>())
		{
			if (ChatTeam->GetText().EqualTo(FText::FromString(TEXT("[ALL]"))))
			{
				PS->ReceiveMessage(EShooterTeam::None, PS->GetName(), Text.ToString());
			}
			else
			{
				PS->ReceiveMessage(PS->Team, PS->GetName(), Text.ToString());
			}

			UWidgetBlueprintLibrary::SetInputMode_GameOnly(UShooterBPLibrary::GetShooterController(this, 0));
			ChatTextBox->SetText(FText::FromString(TEXT("")));
			ChatEntry->SetVisibility(ESlateVisibility::Hidden);
		}
		else if (AZombieShooterPlayerState* ZPS = GetOwningPlayerState<AZombieShooterPlayerState>())
		{
			ZPS->ReceiveMessage(EZombieShooterTeam::None, ZPS->GetName(), Text.ToString());

			UWidgetBlueprintLibrary::SetInputMode_GameOnly(UZombieShooterBPLibrary::GetZombieShooterController(this, 0));
			ChatTextBox->SetText(FText::FromString(TEXT("")));
			ChatEntry->SetVisibility(ESlateVisibility::Hidden);
		}
	

	}
}

void UShooterBulletCounterUI::DoOnCheckStateChanged(bool IsChecked)
{
	if (AShooterPlayerState* PS = GetOwningPlayerState<AShooterPlayerState>())
	{
		PS->UpdateReady(IsChecked);
	}
	else if(AZombieShooterPlayerState* ZPS = GetOwningPlayerState<AZombieShooterPlayerState>())
	{
		ZPS->UpdateReady(IsChecked);
	}
}

void UShooterBulletCounterUI::ShowSaveIndicator()
{
	SaveIndicator->SetVisibility(ESlateVisibility::Visible);
	SIOnScreen = true;
}

void UShooterBulletCounterUI::PlaySpreeSound_Implementation(USoundBase* Sound)
{
	UGameplayStatics::PlaySound2D(this, Sound);
}




