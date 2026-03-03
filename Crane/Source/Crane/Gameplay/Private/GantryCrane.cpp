#include "Gameplay/Public/GantryCrane.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"

AGantryCrane::AGantryCrane()
{
	PrimaryActorTick.bCanEverTick = true;

	CraneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CraneRoot"));
	SetRootComponent(CraneRoot);

	BridgeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BridgeMesh"));
	BridgeMesh->SetupAttachment(CraneRoot);
	BridgeMesh->SetSimulatePhysics(false);

	TrolleyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrolleyMesh"));
	TrolleyMesh->SetupAttachment(BridgeMesh);

	HookMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HookMesh"));
	HookMesh->SetupAttachment(TrolleyMesh);

	PossessVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("PossessVolume"));
	PossessVolume->SetupAttachment(CraneRoot);

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
}

void AGantryCrane::BeginPlay()
{
	Super::BeginPlay();
	
	BridgeWorldPos = BridgeMesh->GetComponentLocation();
	TrolleyRelPos = FVector::ZeroVector;
	HookRelZ = HookMaxZ;

	PossessVolume->OnComponentBeginOverlap.AddDynamic(this, &AGantryCrane::OnPossessVolumeOverlap);
	HookMesh->SetRelativeLocation(FVector(0.f, 0.f, HookRelZ));
}

void AGantryCrane::ApplyPhysics(float& Velocity, float Input, float Accel,
                                  float MaxSpeed, float Damping, float DeltaTime)
{
	if (!FMath::IsNearlyZero(Input))
	{
		Velocity += Input * Accel * DeltaTime;
		Velocity = FMath::Clamp(Velocity, -MaxSpeed, MaxSpeed);
	}
	else
	{
		Velocity = FMath::FInterpTo(Velocity, 0.f, DeltaTime, Damping);
	}
}

void AGantryCrane::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//bridge
	ApplyPhysics(BridgeVelocity, BridgeInput,BridgeAcceleration, BridgeMaxSpeed, BridgeDamping, DeltaTime);

	BridgeWorldPos.X += BridgeVelocity * DeltaTime;
	BridgeWorldPos.X = FMath::Clamp(BridgeWorldPos.X,GetActorLocation().X - BridgeRailHalfLength,GetActorLocation().X + BridgeRailHalfLength);
	BridgeMesh->SetWorldLocation(BridgeWorldPos);

	//trolley
	ApplyPhysics(TrolleyVelocity, TrolleyInput,TrolleyAcceleration, TrolleyMaxSpeed, TrolleyDamping, DeltaTime);

	TrolleyRelPos.Y += TrolleyVelocity * DeltaTime;
	TrolleyRelPos.Y = FMath::Clamp(TrolleyRelPos.Y, -TrolleyHalfLength, TrolleyHalfLength);
	TrolleyMesh->SetRelativeLocation(TrolleyRelPos);

	//Hook Winch
	float WinchDir = bLoweringHook? -1.f: 1.f; // -Z = down
	HookRelZ += WinchDir * WinchSpeed * DeltaTime;
	HookRelZ = FMath::Clamp(HookRelZ, HookMinZ, HookMaxZ);
	HookMesh->SetRelativeLocation(FVector(0.f, 0.f, HookRelZ));
}

void AGantryCrane::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC) return;

	if (IA_Move)
	{
		EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AGantryCrane::OnMove);
	}

	if (IA_Winch)
	{
		EIC->BindAction(IA_Winch, ETriggerEvent::Started, this, &AGantryCrane::OnWinchPressed);
		EIC->BindAction(IA_Winch, ETriggerEvent::Completed, this, &AGantryCrane::OnWinchReleased);
	}
	if (IA_Unpossess)
		EIC->BindAction(IA_Unpossess, ETriggerEvent::Started, this, &AGantryCrane::OnUnpossess);
}

void AGantryCrane::OnMove(const FInputActionValue& Value)
{
	FVector2D MoveAxis = Value.Get<FVector2D>();
	BridgeInput  = MoveAxis.Y;// W/S = bridge forward/back
	TrolleyInput = MoveAxis.X;// A/D = trolley left/right
}

void AGantryCrane::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	APlayerController* PC = Cast<APlayerController>(NewController);
	if (PC)
	{
		PreviousPawn = PC->GetPawn();
	}
}

void AGantryCrane::OnPossessVolumeOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                          bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* OverlappingPawn = Cast<APawn>(OtherActor);
	if (!OverlappingPawn) return;

	APlayerController* PC = Cast<APlayerController>(OverlappingPawn->GetController());
	if (!PC) return;

	APawn* CachedPawn = OverlappingPawn;
	PC->Possess(this);
	PreviousPawn = CachedPawn;
	if (ULocalPlayer* LP = PC->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (IMC_Crane)
			{
				Subsystem->AddMappingContext(IMC_Crane, 1);
			}
		}
	}
}

void AGantryCrane::OnUnpossess()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC && PreviousPawn)
	{
		PC->Possess(PreviousPawn);
		PreviousPawn = nullptr;
	}
}

void AGantryCrane::OnBridgeForward(float Val)
{
	BridgeInput = Val;
}

void AGantryCrane::OnTrolleyRight(float Val)
{
	TrolleyInput = Val;
}

void AGantryCrane::OnWinchPressed()
{
	bLoweringHook = true;
}

void AGantryCrane::OnWinchReleased()
{
	bLoweringHook = false;
}
