#include "CGGameMode.h"
#include "CGShip.h"
#include "TimerManager.h"

ACGGameMode::ACGGameMode()
{
}

void ACGGameMode::BeginPlay()
{
	Super::BeginPlay();
	SpawnShip();
}

void ACGGameMode::SetState(EShipState NewState)
{
	CurrentState = NewState;
	UE_LOG(LogTemp, Warning, TEXT("ShipState -> %s"),
		CurrentState == EShipState::Docked   ? TEXT("Docked") :
		CurrentState == EShipState::Sailing   ? TEXT("Sailing") :
												TEXT("Spawning"));
}

void ACGGameMode::SendShipOff()
{
	if (CurrentState != EShipState::Docked) return;
	if (!CurrentShip) return;

	SetState(EShipState::Sailing);
	CurrentShip->Depart();

	//Wait for ship to despawn then spawn a new one
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ACGGameMode::OnShipDespawned,
		CurrentShip->DespawnDelay + 0.5f, false);
}

void ACGGameMode::OnShipDespawned()
{
	//do rewards here
	CurrentShip = nullptr;
	SetState(EShipState::Spawning);

	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ACGGameMode::SpawnShip,
		SpawnDelay, false);
}

void ACGGameMode::SpawnShip()
{
	if (!ShipClass) 
	{
		UE_LOG(LogTemp, Warning, TEXT("ACGGameMode: ShipClass is not set!"));
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	CurrentShip = GetWorld()->SpawnActor<ACGShip>(ShipClass, ShipSpawnTransform, Params);
	SetState(EShipState::Docked);
}