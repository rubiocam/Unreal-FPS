// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AmmoPickup.generated.h"

UCLASS()
class MULTI_API AAmmoPickup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAmmoPickup();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	TObjectPtr<class USphereComponent> BulletSphere;
	TObjectPtr<class UTextRenderComponent> TextRenderer;

	UPROPERTY(EditAnywhere)
	int AmmoAmount = 5;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};
