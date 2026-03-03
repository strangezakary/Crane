#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CGGameMode.generated.h"

UENUM(BlueprintType)
enum class EShipState : uint8
{
	Docked      UMETA(DisplayName = "Docked"),
	Sailing     UMETA(DisplayName = "Sailing"),
	Spawning    UMETA(DisplayName = "Spawning")
};

class ACGShip;

UCLASS()
class ACGGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACGGameMode();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Ship")
	void SendShipOff();

	UFUNCTION(BlueprintCallable, Category = "Ship")
	EShipState GetShipState() const { return CurrentState; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	TSubclassOf<ACGShip> ShipClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	FTransform ShipSpawnTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship")
	float SpawnDelay = 3.f;

private:
	EShipState CurrentState = EShipState::Docked;

	UPROPERTY()
	TObjectPtr<ACGShip> CurrentShip;

	void SpawnShip();
	void OnShipDespawned();
	void SetState(EShipState NewState);

	FTimerHandle SpawnTimerHandle;
};