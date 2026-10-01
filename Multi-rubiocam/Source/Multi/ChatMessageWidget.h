// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)

#pragma once

#include "CoreMinimal.h"
#include "ShooterPlayerState.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Variant_Zombie/ZombieShooterPlayerState.h"
#include "ChatMessageWidget.generated.h"

/**
 * 
 */
UCLASS()
class MULTI_API UChatMessageWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetChatMessage(EShooterTeam Team, const FString& FSSender, const FString& FSMessage);
	void SetChatMessageZVer(EZombieShooterTeam Team, const FString& FSSender, const FString& FSMessage);
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Channel;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Sender;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Message;
private:
	
protected:
};
