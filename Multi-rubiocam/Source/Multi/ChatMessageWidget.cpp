// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)


#include "ChatMessageWidget.h"

void UChatMessageWidget::SetChatMessage(EShooterTeam Team, const FString& FSSender, const FString& FSMessage)
{
	if (Team == EShooterTeam::Blue)
	{
		Channel->SetText(FText::FromString(TEXT("[BLUE]")));
		Channel->SetColorAndOpacity(FLinearColor::Blue);
	}
	else if (Team == EShooterTeam::Red)
	{
		Channel->SetText(FText::FromString(TEXT("[RED]")));
		Channel->SetColorAndOpacity(FLinearColor::Red);
	}
	else
	{
		Channel->SetText(FText::FromString(TEXT("[ALL]")));
		Channel->SetColorAndOpacity(FLinearColor::White);
	}
	
	Sender->SetText(FText::FromString(FSSender));
	Message->SetText(FText::FromString(FSMessage));
}

void UChatMessageWidget::SetChatMessageZVer(EZombieShooterTeam Team, const FString& FSSender, const FString& FSMessage)
{
	if (Team == EZombieShooterTeam::Blue)
	{
		Channel->SetText(FText::FromString(TEXT("[BLUE]")));
		Channel->SetColorAndOpacity(FLinearColor::Blue);
	}
	else if (Team == EZombieShooterTeam::Red)
	{
		Channel->SetText(FText::FromString(TEXT("[RED]")));
		Channel->SetColorAndOpacity(FLinearColor::Red);
	}
	else
	{
		Channel->SetText(FText::FromString(TEXT("[ALL]")));
		Channel->SetColorAndOpacity(FLinearColor::White);
	}
	
	Sender->SetText(FText::FromString(FSSender));
	Message->SetText(FText::FromString(FSMessage));
}
