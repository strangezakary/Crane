#include "Public/GantryCrane.h"
#include "Public/CGCrate.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"

AGantryCrane::AGantryCrane()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    CraneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CraneRoot"));
    SetRootComponent(CraneRoot);

    BridgeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BridgeMesh"));
    BridgeMesh->SetupAttachment(CraneRoot);
    BridgeMesh->SetSimulatePhysics(false);
    BridgeMesh->SetIsReplicated(true);

    TrolleyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrolleyMesh"));
    TrolleyMesh->SetupAttachment(BridgeMesh);
    TrolleyMesh->SetIsReplicated(true);

    HookMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HookMesh"));
    HookMesh->SetupAttachment(TrolleyMesh);
    HookMesh->SetIsReplicated(true);

    HookVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("HookVolume"));
    HookVolume->SetupAttachment(HookMesh);
    HookVolume->SetBoxExtent(FVector(50.f, 50.f, 50.f));
    HookVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

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

void AGantryCrane::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AGantryCrane, BridgeWorldPos);
    DOREPLIFETIME(AGantryCrane, TrolleyRelPos);
    DOREPLIFETIME(AGantryCrane, HookRelZ);
    DOREPLIFETIME(AGantryCrane, HeldCrate);
    DOREPLIFETIME(AGantryCrane, PreviousPawn);
}

void AGantryCrane::BeginPlay()
{
    Super::BeginPlay();

    BridgeWorldPos = BridgeMesh->GetComponentLocation();
    TrolleyRelPos = FVector::ZeroVector;
    HookRelZ = HookMaxZ;

    if (HasAuthority())
    {
        PossessVolume->OnComponentBeginOverlap.AddDynamic(this, &AGantryCrane::OnPossessVolumeOverlap);
        HookVolume->OnComponentBeginOverlap.AddDynamic(this, &AGantryCrane::OnHookVolumeOverlap);
    }

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

    if (HasAuthority())
    {
        ApplyPhysics(BridgeVelocity, BridgeInput, BridgeMaxSpeed, BridgeDamping, DeltaTime);
        BridgeWorldPos.X += BridgeVelocity * DeltaTime;
        BridgeWorldPos.X = FMath::Clamp(
            BridgeWorldPos.X,
            GetActorLocation().X - BridgeRailHalfLength,
            GetActorLocation().X + BridgeRailHalfLength);

        ApplyPhysics(TrolleyVelocity, TrolleyInput, TrolleyMaxSpeed, TrolleyDamping, DeltaTime);
        TrolleyRelPos.Y += TrolleyVelocity * DeltaTime;
        TrolleyRelPos.Y = FMath::Clamp(TrolleyRelPos.Y, -TrolleyHalfLength, TrolleyHalfLength);

        const float WinchDir = bLoweringHook ? -1.f : 1.f;
        HookRelZ += WinchDir * WinchSpeed * DeltaTime;
        HookRelZ = FMath::Clamp(HookRelZ, HookMinZ, HookMaxZ);
    }

    BridgeMesh->SetWorldLocation(BridgeWorldPos);
    TrolleyMesh->SetRelativeLocation(TrolleyRelPos);
    HookMesh->SetRelativeLocation(FVector(0.f, 0.f, HookRelZ));
}

void AGantryCrane::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!EIC)
    {
        return;
    }

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
        EIC->BindAction(IA_Winch, ETriggerEvent::Started,   this, &AGantryCrane::OnWinchPressed);
        EIC->BindAction(IA_Winch, ETriggerEvent::Completed, this, &AGantryCrane::OnWinchReleased);
    }

    if (IA_Unpossess)
    {
        EIC->BindAction(IA_Unpossess, ETriggerEvent::Started, this, &AGantryCrane::OnUnpossess);
    }

    if (IA_Drop)
    {
        EIC->BindAction(IA_Drop, ETriggerEvent::Started, this, &AGantryCrane::OnDrop);
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
    Server_SetMoveInput(MoveAxis.Y, MoveAxis.X);
}

void AGantryCrane::OnMoveStop(const FInputActionValue& Value)
{
    Server_SetMoveInput(0.f, 0.f);
}

void AGantryCrane::Server_SetMoveInput_Implementation(float Bridge, float Trolley)
{
    BridgeInput  = Bridge;
    TrolleyInput = Trolley;
}

void AGantryCrane::OnWinchPressed()
{
    Server_SetWinch(true);
}

void AGantryCrane::OnWinchReleased()
{
    Server_SetWinch(false);
}

void AGantryCrane::Server_SetWinch_Implementation(bool bLowering)
{
    bLoweringHook = bLowering;
}

void AGantryCrane::OnHookVolumeOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority())
    {
        return;
    }

    if (HeldCrate)
    {
        return;
    }

    ACGCrate* Crate = Cast<ACGCrate>(OtherActor);
    if (!Crate)
    {
        return;
    }

    HeldCrate = Crate;

    UPrimitiveComponent* CrateRoot = Cast<UPrimitiveComponent>(Crate->GetRootComponent());
    if (CrateRoot)
    {
        CrateRoot->SetSimulatePhysics(false);
    }

    Crate->AttachToComponent(HookMesh, FAttachmentTransformRules::KeepWorldTransform);
}

void AGantryCrane::OnDrop(const FInputActionValue& Value)
{
    Server_Drop();
}

void AGantryCrane::Server_Drop_Implementation()
{
    if (!HeldCrate)
    {
        return;
    }

    HeldCrate->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    UPrimitiveComponent* CrateRoot = Cast<UPrimitiveComponent>(HeldCrate->GetRootComponent());
    if (CrateRoot)
    {
        CrateRoot->SetSimulatePhysics(true);
    }

    HeldCrate = nullptr;
}

void AGantryCrane::OnPossessVolumeOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority())
    {
        return;
    }

    APawn* OverlappingPawn = Cast<APawn>(OtherActor);
    if (!OverlappingPawn)
    {
        return;
    }

    APlayerController* PC = Cast<APlayerController>(OverlappingPawn->GetController());
    if (!PC)
    {
        return;
    }

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
    Server_Unpossess();
}

void AGantryCrane::Server_Unpossess_Implementation()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC || !PreviousPawn)
    {
        return;
    }

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