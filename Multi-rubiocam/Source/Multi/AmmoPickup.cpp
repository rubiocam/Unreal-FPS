// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)


#include "AmmoPickup.h"

#include "ShooterCharacter.h"
#include "ShooterWeapon.h"
#include "Components/SphereComponent.h"
#include "Components/TextRenderComponent.h"
#include "Variant_Zombie/ZombieShooterCharacter.h"

// Sets default values
AAmmoPickup::AAmmoPickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);

	BulletSphere = CreateDefaultSubobject<USphereComponent>("BulletSphere");
	TextRenderer = CreateDefaultSubobject<UTextRenderComponent>("TextRender");

	SetRootComponent(BulletSphere);
	TextRenderer->SetupAttachment(RootComponent);
	TextRenderer->SetText(FText::FromString(FString("Ammo")));
	
	TextRenderer->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	TextRenderer->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
}

// Called when the game starts or when spawned
void AAmmoPickup::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAmmoPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAmmoPickup::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (GetLocalRole() == ROLE_Authority)
	{
		if (AShooterCharacter* Char = Cast<AShooterCharacter>(OtherActor))
		{
			if (AShooterWeapon* Weapon = Char->GetCurrentWeapon())
			{
				if (!Weapon->FullOfAmmo())
				{
					Weapon->AddAmmo(AmmoAmount);

					Destroy();
				}
				
			}
		}
		else if (AZombieShooterCharacter* ZChar = Cast<AZombieShooterCharacter>(OtherActor))
		{
			if (AShooterWeapon* Weapon = Char->GetCurrentWeapon())
			{
				if (!Weapon->FullOfAmmo())
				{
					Weapon->AddAmmo(AmmoAmount);

					Destroy();
				}
				
			}
		}
	}
}

