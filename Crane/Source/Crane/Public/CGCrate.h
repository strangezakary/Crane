// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CGItem.h"
#include "CGManifest.h"
#include "CGCrate.generated.h"

class UStaticMeshComponent;

#define EMPTY_CRATE_WEIGHT_KG 4000.0f

// NOTE(zak): Crates MUST have a manifest to properly spawn in. 
// If you are spawning this at runtime via SpawnActor, you should instead use
// Spawn Actor deferred and then set the Manifest on the actor before calling FinalizeSpawning
// In BP you can just make a manifest by hand and set it on the BP
UCLASS()
class CRANE_API ACGCrate : public AActor
{
	GENERATED_BODY()
	
public:	
	ACGCrate();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	
	void SetManifest(UCrateManifest* InManifest) 
	{
		Manifest = InManifest;
	}

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh = nullptr;

	UPROPERTY(EditAnywhere)
	UCrateManifest* Manifest = nullptr;

	UPROPERTY(EditAnywhere)
	float CrateWeight = EMPTY_CRATE_WEIGHT_KG; // Different sized crates should be different weights 
};
