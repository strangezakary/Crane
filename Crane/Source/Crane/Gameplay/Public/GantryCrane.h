#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
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
    void PossessedBy(AController* NewController);
    void OnUnpossess();
	void OnMove(const FInputActionValue& Value);

	UFUNCTION()
	void OnPossessVolumeOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crane|Components")
    UStaticMeshComponent* BridgeMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crane|Components")
    UStaticMeshComponent* TrolleyMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crane|Components")
    UStaticMeshComponent* HookMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crane|Components")
    USceneComponent* CraneRoot;

	UPROPERTY()
	TObjectPtr<APawn> PreviousPawn;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crane|Components")
	TObjectPtr<UBoxComponent> PossessVolume;

	UPROPERTY(EditAnywhere, Category = "Crane|Input")
	TObjectPtr<UInputMappingContext> IMC_Crane;

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
    float BridgeRailHalfLength = 1000.f; //limit

    //Trolley
    UPROPERTY(EditAnywhere, Category = "Crane|Trolley")
    float TrolleyAcceleration = 250.f;

    UPROPERTY(EditAnywhere, Category = "Crane|Trolley")
    float TrolleyMaxSpeed = 400.f;

    UPROPERTY(EditAnywhere, Category = "Crane|Trolley")
    float TrolleyDamping = 4.f;

    UPROPERTY(EditAnywhere, Category = "Crane|Trolley")
    float TrolleyHalfLength = 600.f; // Relative to bridge center ±Y

    //Hook (Space)
    UPROPERTY(EditAnywhere, Category = "Crane|Hook")
    float WinchSpeed = 350.f; //cm/s

    UPROPERTY(EditAnywhere, Category = "Crane|Hook")
    float HookMinZ = -800.f; //Lowest relative position

    UPROPERTY(EditAnywhere, Category = "Crane|Hook")
    float HookMaxZ = 0.f; //highest (retracted)

private:
    float BridgeVelocity = 0.f; //along X
    float TrolleyVelocity = 0.f; //along Y 

    float BridgeInput = 0.f; //-1 to 1, from W/S
    float TrolleyInput = 0.f; //-1 to 1, from A/D
    bool bLoweringHook = false; 

    FVector BridgeWorldPos;
    FVector TrolleyRelPos;
    float HookRelZ;

    void ApplyPhysics(float& Velocity, float Input, float Accel, float MaxSpeed, float Damping, float DeltaTime);

    void OnBridgeForward(float Val);
    void OnTrolleyRight(float Val);
    void OnWinchPressed();
    void OnWinchReleased();
};
