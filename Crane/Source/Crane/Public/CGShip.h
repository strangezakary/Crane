#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CGShip.generated.h"

UCLASS()
class ACGShip : public AActor
{
	GENERATED_BODY()

public:
	ACGShip();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	void Depart();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship|Components")
	TObjectPtr<UStaticMeshComponent> ShipMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Movement")
	FVector DepartureDirection = FVector(1.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Movement")
	float DepartureForce = 500000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship|Movement")
	float DespawnDelay = 5.f;

private:
	bool bDeparting = false;
	void OnDespawnTimer();
};