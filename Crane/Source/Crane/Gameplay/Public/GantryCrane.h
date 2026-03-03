
#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "GantryCrane.generated.h"

class UInputMappingContext;

UCLASS()
class AGantryCrane : public APawn
{
	GENERATED_BODY()

public:
	AGantryCrane();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void OnUnpossess();
	void OnMove(const FInputActionValue& Value);
	void OnMoveStop(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);

	UFUNCTION()
	void OnPossessVolumeOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crane|Components")
	TObjectPtr<USceneComponent> CraneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crane|Components")
	TObjectPtr<UStaticMeshComponent> BridgeMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crane|Components")
	TObjectPtr<UStaticMeshComponent> TrolleyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crane|Components")
	TObjectPtr<UStaticMeshComponent> HookMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crane|Components")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crane|Components")
	TObjectPtr<UCameraComponent> CraneCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crane|Components")
	TObjectPtr<UBoxComponent> PossessVolume;

	UPROPERTY()
	TObjectPtr<APawn> PreviousPawn;

	UPROPERTY(EditAnywhere, Category = "Crane|Input")
	TObjectPtr<UInputMappingContext> IMC_Crane;

	UPROPERTY(EditAnywhere, Category = "Crane|Input")
	TObjectPtr<UInputAction> IA_Look;

	UPROPERTY(EditAnywhere, Category = "Crane|Input")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditAnywhere, Category = "Crane|Input")
	TObjectPtr<UInputAction> IA_Winch;

	UPROPERTY(EditAnywhere, Category = "Crane|Input")
	TObjectPtr<UInputAction> IA_Unpossess;

	//Bridge
	UPROPERTY(EditAnywhere, Category = "Crane|Bridge")
	float BridgeAcceleration = 300.f;

	UPROPERTY(EditAnywhere, Category = "Crane|Bridge")
	float BridgeMaxSpeed = 500.f;

	UPROPERTY(EditAnywhere, Category = "Crane|Bridge")
	float BridgeDamping = 4.f;

	UPROPERTY(EditAnywhere, Category = "Crane|Bridge")
	float BridgeRailHalfLength = 1000.f;

	//Trolley
	UPROPERTY(EditAnywhere, Category = "Crane|Trolley")
	float TrolleyAcceleration = 250.f;

	UPROPERTY(EditAnywhere, Category = "Crane|Trolley")
	float TrolleyMaxSpeed = 400.f;

	UPROPERTY(EditAnywhere, Category = "Crane|Trolley")
	float TrolleyDamping = 4.f;

	UPROPERTY(EditAnywhere, Category = "Crane|Trolley")
	float TrolleyHalfLength = 600.f;

	//Hook
	UPROPERTY(EditAnywhere, Category = "Crane|Hook")
	float WinchSpeed = 350.f;

	UPROPERTY(EditAnywhere, Category = "Crane|Hook")
	float HookMinZ = -800.f;

	UPROPERTY(EditAnywhere, Category = "Crane|Hook")
	float HookMaxZ = 0.f;

private:
	float BridgeVelocity = 0.f;
	float TrolleyVelocity = 0.f;
	float BridgeInput = 0.f;
	float TrolleyInput = 0.f;
	bool bLoweringHook = false;

	FVector BridgeWorldPos = FVector::ZeroVector;
	FVector TrolleyRelPos = FVector::ZeroVector;
	float HookRelZ = 0.f;

	void ApplyPhysics(float& Velocity, float Input, float MaxSpeed, float Damping, float DeltaTime);

	void OnBridgeForward(float Val);
	void OnTrolleyRight(float Val);
	void OnWinchPressed();
	void OnWinchReleased();
};