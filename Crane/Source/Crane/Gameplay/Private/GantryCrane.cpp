
#include "Gameplay/Public/GantryCrane.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"

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

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(CraneRoot);
	SpringArm->TargetArmLength = 1200.f;
	SpringArm->SetRelativeLocation(FVector(0.f, 0.f, 200.f));
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bInheritPitch = true;
	SpringArm->bInheritYaw = true;
	SpringArm->bInheritRoll = false;

	CraneCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CraneCamera"));
	CraneCamera->SetupAttachment(SpringArm);
	CraneCamera->bUsePawnControlRotation = false;

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

void AGantryCrane::ApplyPhysics(float& Velocity, float Input, float MaxSpeed, float Damping, float DeltaTime)
{
	if (!FMath::IsNearlyZero(Input))
	{
		Velocity = Input * MaxSpeed;
	}
	else
	{
		Velocity = FMath::FInterpTo(Velocity, 0.f, DeltaTime, Damping);
	}
		
}

void AGantryCrane::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Bridge
	ApplyPhysics(BridgeVelocity, BridgeInput, BridgeMaxSpeed, BridgeDamping, DeltaTime);

	BridgeWorldPos.X += BridgeVelocity * DeltaTime;
	BridgeWorldPos.X = FMath::Clamp(
		BridgeWorldPos.X,
		GetActorLocation().X - BridgeRailHalfLength,
		GetActorLocation().X + BridgeRailHalfLength);

	BridgeMesh->SetWorldLocation(BridgeWorldPos);

	//Trolley
	ApplyPhysics(TrolleyVelocity, TrolleyInput, TrolleyMaxSpeed, TrolleyDamping, DeltaTime);

	TrolleyRelPos.Y += TrolleyVelocity * DeltaTime;
	TrolleyRelPos.Y = FMath::Clamp(TrolleyRelPos.Y, -TrolleyHalfLength, TrolleyHalfLength);
	TrolleyMesh->SetRelativeLocation(TrolleyRelPos);

	//Hook Winch
	const float WinchDir = bLoweringHook ? -1.f : 1.f; // -Z = down
	HookRelZ += WinchDir * WinchSpeed * DeltaTime;
	HookRelZ = FMath::Clamp(HookRelZ, HookMinZ, HookMaxZ);
	HookMesh->SetRelativeLocation(FVector(0.f, 0.f, HookRelZ));
}

void AGantryCrane::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC) return;
	
	if (IA_Look)
	{
		EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AGantryCrane::OnLook);
	}
	
	if (IA_Move)
	{
		EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AGantryCrane::OnMove);
		EIC->BindAction(IA_Move, ETriggerEvent::Completed, this, &AGantryCrane::OnMoveStop);
	}

	if (IA_Winch)
	{
		EIC->BindAction(IA_Winch, ETriggerEvent::Started, this, &AGantryCrane::OnWinchPressed);
		EIC->BindAction(IA_Winch, ETriggerEvent::Completed, this, &AGantryCrane::OnWinchReleased);
	}

	if (IA_Unpossess)
	{
		EIC->BindAction(IA_Unpossess, ETriggerEvent::Started, this, &AGantryCrane::OnUnpossess);
	}
}

void AGantryCrane::OnLook(const FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>();
	AddControllerYawInput(LookAxis.X);
	AddControllerPitchInput(LookAxis.Y);
}

void AGantryCrane::OnMove(const FInputActionValue& Value)
{
	const FVector2D MoveAxis = Value.Get<FVector2D>();
	BridgeInput = MoveAxis.Y;  //W/S = bridge forward/back
	TrolleyInput = MoveAxis.X; //A/D = trolley left/right
}

void AGantryCrane::OnMoveStop(const FInputActionValue& Value)
{
	BridgeInput  = 0.f;
	TrolleyInput = 0.f;
}

void AGantryCrane::OnPossessVolumeOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	APawn* OverlappingPawn = Cast<APawn>(OtherActor);
	if (!OverlappingPawn) return;

	APlayerController* PC = Cast<APlayerController>(OverlappingPawn->GetController());
	if (!PC) return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP ? LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;
	
	PC->SetInputMode(FInputModeGameOnly());
	PC->bShowMouseCursor = false;

	if (Subsystem)
	{
		Subsystem->ClearAllMappings();
	}

	APawn* CachedPawn = OverlappingPawn;
	PC->Possess(this);
	PreviousPawn = CachedPawn;

	if (Subsystem && IMC_Crane)
	{
		Subsystem->AddMappingContext(IMC_Crane, 0);
	}
}

void AGantryCrane::OnUnpossess()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || !PreviousPawn) return;
	
	PC->Possess(PreviousPawn);
	PreviousPawn = nullptr;
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