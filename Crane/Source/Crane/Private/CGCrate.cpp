// Fill out your copyright notice in the Description page of Project Settings.


#include "CGCrate.h"
#include "Components/StaticMeshComponent.h"


ACGCrate::ACGCrate()
{
	PrimaryActorTick.bCanEverTick = true;
		
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrateMesh"));
	Mesh->SetSimulatePhysics(true);
}

void ACGCrate::BeginPlay()
{
	// We need to do this in begin play as we dont have the manifest in the constructor 
	check(Manifest);

	float TotalItemWeight = 0.0f;
	for (auto& Item : Manifest->Items)
	{
		TotalItemWeight += Item.Weight;
	}

	check(!FMath::IsNearlyZero(TotalItemWeight));

	float TotalWeight = TotalItemWeight + CrateWeight;
	Mesh->SetMassOverrideInKg(NAME_None, TotalWeight);
}

// Called every frame
void ACGCrate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

