// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCameraPawn.h"
#include "Engine/CollisionProfile.h"
#include "Components/SphereComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/PlayerController.h"
#include "Math/UnrealMathUtility.h"
#include "Camera/CameraComponent.h"

#pragma optimize("", off)
// Sets default values
APlayerCameraPawn::APlayerCameraPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent0"));
	CollisionComponent->InitSphereRadius(35.0f);
	CollisionComponent->SetShouldUpdatePhysicsVolume(true);
	CollisionComponent->SetCanEverAffectNavigation(false);
	CollisionComponent->bDynamicObstacle = true;

	RootComponent = CollisionComponent;

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent0"));
	ArrowComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	ArrowComponent0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent1"));
	ArrowComponent0->AttachToComponent(ArrowComponent, FAttachmentTransformRules::KeepRelativeTransform);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent0"));
	CameraComponent->AttachToComponent(ArrowComponent0, FAttachmentTransformRules::KeepRelativeTransform);

	MoveSpeed = 1.0;

	PlanetActor = nullptr;
}

// Called when the game starts or when spawned
void APlayerCameraPawn::BeginPlay()
{
	LastUpdatePlanetPoint = { 0,0,0 };
	Elevation = GetActorLocation().Size();
	Super::BeginPlay();
}

// Called every frame
void APlayerCameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (TargetPoint != FVector(0.0, 0.0, 0.0 ))
	{
		FVector ActorLocation = GetActorLocation();
		FVector TargetNormalize = TargetPoint - ActorLocation;
		float Size = TargetNormalize.Size();
		if ( Size < MoveSpeed)
		{
			ModifyLocation(TargetPoint);
			TargetPoint = {0.0, 0.0, 0.0};
		}
		else
		{
			TargetNormalize.Normalize();
			FVector Position = TargetNormalize * MoveSpeed + GetActorLocation();
			ModifyLocation(Position);
		}
		if (Elevation > 600000)
		{

		}
	}

	if (LastLocation != GetActorLocation())
	{
		SetActorRotation(FRotationMatrix::MakeFromX(GetActorLocation() * -1.0).Rotator());
		LastLocation = GetActorLocation();
	}

	float Sc = Elevation - PlanetActor->GetPlanetRadius();
	Sc = FMath::Clamp(Sc, UpdateHeightMin, UpdateHeightMax);
	float S = 1.0 - pow((Sc - UpdateHeightMin) / UpdateHeightMax, PowParam);
	float V = FMath::Clamp(S, 0.1f, 1.0f) * Sc;
	float Dis = (LastUpdatePlanetPoint - GetActorLocation()).Size();
	if (V < Dis)
	{
		PlanetActor->AsyncUpdateTerrain();
		LastUpdatePlanetPoint = GetActorLocation();
	}
}

// Called to bind functionality to input
void APlayerCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCameraPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveUp", this, &APlayerCameraPawn::MoveUp);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCameraPawn::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APlayerCameraPawn::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerCameraPawn::LookUp);
	PlayerInputComponent->BindAction("AddMoveSpeed", IE_Pressed, this, &APlayerCameraPawn::AddMoveSpeed);
	PlayerInputComponent->BindAction("SubMoveSpeed", IE_Pressed, this, &APlayerCameraPawn::SubMoveSpeed);

}
void APlayerCameraPawn::MoveRight(float Val)
{
	if (Val != 0.f)
	{
		TargetPoint = {0.0, 0.0, 0.0};
		FVector Position = CameraComponent->GetRightVector() * Val * MoveSpeed + GetActorLocation();
		ModifyLocation(Position, false);
	}
}

void APlayerCameraPawn::MoveForward(float Val)
{
	if (Val != 0.f)
	{
		TargetPoint = {0.0, 0.0, 0.0};
		FVector Position = CameraComponent->GetForwardVector() * Val * MoveSpeed + GetActorLocation();
		ModifyLocation(Position, false);
	}
}
void APlayerCameraPawn::Turn(float Val)
{
	if (Val != 0.f)
	{
		ArrowComponent->AddRelativeRotation({0, 0, Val});
	}
}
void APlayerCameraPawn::LookUp(float Val)
{
	if (Val != 0.f)
	{
		CameraComponent->AddRelativeRotation({-Val, 0, 0});
	}
}
void APlayerCameraPawn::AddMoveSpeed()
{
	MoveSpeedScale = MoveSpeedScale * 2.0;

}

void APlayerCameraPawn::SubMoveSpeed()
{
	MoveSpeedScale = MoveSpeedScale / 2.0;
}
void APlayerCameraPawn::MoveUp(float Val)
{
	if (Val != 0.f)
	{
		TargetPoint = {0.0, 0.0, 0.0};
		FVector Vector = GetActorLocation();
		Vector.Normalize();
		FVector Position = Vector * Val * MoveSpeed + GetActorLocation();
		ModifyLocation(Position);
	}

}
void APlayerCameraPawn::ModifyLocation(const FVector &Pos, bool IsModifyHeight)
{
	if (PlanetActor == nullptr)
	{
		SetActorLocation(Pos);
		return;
	}
	FVector Position = Pos;
	FVector PosNormal = Pos;
	PosNormal.Normalize();

	if (PlanetActor)
	{
		if (!IsModifyHeight)
		{
			Position = PosNormal * Elevation;
		}

		float Height = PlanetActor->GetTerrainHeight(PosNormal);

		if ((Height + MinAltitude) > Elevation)
		{
			Position = PosNormal * (Height + MinAltitude);
			Elevation = Height + MinAltitude;
		}

		if (IsModifyHeight)
		{
			Elevation = Pos.Size();
		}

		MoveSpeed = (Elevation - PlanetActor->GetPlanetRadius()) * MoveSpeedScale;
		if (MoveSpeed < 0.0)
		{
			MoveSpeed = 1.0;
		}
	}

	SetActorLocation(Position);
	SetActorRotation(FRotationMatrix::MakeFromX(GetActorLocation() * -1.0).Rotator());
}
void APlayerCameraPawn::MovePoint(const FVector &Val) 
{ 
	TargetPoint = Val; 
	if (PlanetActor)
	{
		TargetPoint.Normalize();
		float Height = PlanetActor->GetTerrainHeight(TargetPoint);
		if ((Height + MinAltitude) > Elevation)
		{
			TargetPoint = TargetPoint * (Height + MinAltitude);
		}
		else
		{
			TargetPoint = Val;
		}
	}
};
void APlayerCameraPawn::SetUsingAbsoluteRotation(USceneComponent *SceneComponent, bool bInAbsoluteRotation)
{
	SceneComponent->SetUsingAbsoluteRotation(bInAbsoluteRotation);
}
void APlayerCameraPawn::ChangePlanetMovemount(APlanetActor *Planet)
{
	PlanetActor = Planet;
	if (PlanetActor != nullptr)
	{
		SetActorRotation(FRotationMatrix::MakeFromX(GetActorLocation() * -1.0).Rotator());
		TargetPoint = { 0.0, 0.0, 0.0 };
	}
}
#pragma optimize("", on)
