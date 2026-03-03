#include "CGShip.h"

ACGShip::ACGShip()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	SetRootComponent(ShipMesh);
	ShipMesh->SetSimulatePhysics(true);
	ShipMesh->SetIsReplicated(true);
}

void ACGShip::BeginPlay()
{
	Super::BeginPlay();
}

void ACGShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bDeparting && ShipMesh->IsSimulatingPhysics())
	{
		ShipMesh->AddForce(DepartureDirection.GetSafeNormal() * DepartureForce, NAME_None, true);
	}
}

void ACGShip::Depart()
{
	if (!HasAuthority()) return;
	bDeparting = true;

	FTimerHandle DespawnHandle;
	GetWorldTimerManager().SetTimer(DespawnHandle, this, &ACGShip::OnDespawnTimer, DespawnDelay, false);
}

void ACGShip::OnDespawnTimer()
{
	Destroy();
}