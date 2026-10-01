// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ShooterPlayerState.h"
#include "Blueprint/UserWidget.h"
#include "Components/CheckBox.h"
#include "Components/CircularThrobber.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Variant_Zombie/ZombieShooterPlayerState.h"
#include "ShooterBulletCounterUI.generated.h"

/**
 *  Simple bullet counter UI widget for a first person shooter game
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEditableTextBoxCommittedEvent, const FText&, Text, ETextCommit::Type, CommitMethod);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCheckBoxComponentStateChanged, bool, IsChecked);

UCLASS(abstract)
class MULTI_API UShooterBulletCounterUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	/** Allows Blueprint to update sub-widgets with the new bullet count */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta=(DisplayName = "UpdateBulletCounter"))
	void BP_UpdateBulletCounter(int32 MagazineSize, int32 BulletCount);

	/** Allows Blueprint to update sub-widgets with the new life total and play a damage effect on the HUD */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta=(DisplayName = "Damaged"))
	void BP_Damaged(float LifePercent);

	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> RedScore;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> BlueScore;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> MyScore;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Timer;

	
	UPROPERTY(meta=(BindWidget))
    TObjectPtr<UTextBlock> StartMatchText;

	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> DisplayWinnerText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> SpreeText;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> DeathText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> ChatTextBox;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UScrollBox>ChatScrollBox;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> ChatMessages;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UHorizontalBox> ChatEntry;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ChatTeam;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCheckBox> ReadyCheckBox;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> TagImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCircularThrobber> SaveIndicator;

	UPROPERTY(EditDefaultsOnly)
	TArray<TObjectPtr<UMaterialInterface>> TagMaterials;

	void ChangeDisplayMaterial(int Index);
	
	UPROPERTY(BlueprintAssignable, Category="TextBox|Event")
	FOnEditableTextBoxCommittedEvent OnTextCommitted;

	UPROPERTY(BlueprintAssignable, Category="CheckBox|Event")
	FOnCheckBoxComponentStateChanged OnChecked;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UChatMessageWidget> ChatMessageWidget;
	
	//to show starting match alert
	void ShowAlert(const FString& DisplayText, FLinearColor Color, float Duration);
	//variables for above fcn
	float StartTimer;
	
	void ShowWinnerMessage(FLinearColor Winner);
	void HideWinnerMessage();

	void ShowStreakMessage(class AShooterPlayerState* Killer, class AShooterPlayerState* Killed);
	void HideStreakMessage();

	void ShowDeathTimer(float Duration);
	void HideDeathTimer();
	
	UFUNCTION(NetMulticast, Reliable)
	void PlaySpreeSound(USoundBase* Sound);

	//sends a message to the team of the sender's choice, passing in the sender's name and the message 
	void AddChatMessage(EShooterTeam Team, const FString& Sender, const FString& Message);
	void SendMessage(EShooterTeam Team);

	void AddChatMessageZVer(EZombieShooterTeam Team, const FString& Sender, const FString& Message);
	void SendMessageZVer(EZombieShooterTeam Team);

	UFUNCTION()
	void OnEnter(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void DoOnCheckStateChanged(bool IsChecked);

	void ShowSaveIndicator();

	float SITimer = 0.25f;
	bool SIOnScreen = false;

	
};
