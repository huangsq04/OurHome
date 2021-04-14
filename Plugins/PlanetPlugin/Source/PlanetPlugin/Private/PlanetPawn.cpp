// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanetPawn.h"
#include "Engine/CollisionProfile.h"
#include "Components/SphereComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/PlayerController.h"
#include "Math/UnrealMathUtility.h"
#include "Camera/CameraComponent.h"
#include "FVectorDouble3D.h"
#include "FQuatDouble.h"

#pragma optimize("", off)
// Sets default values
APlanetPawn::APlanetPawn()
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

}

// Called when the game starts or when spawned
void APlanetPawn::BeginPlay()
{
	Super::BeginPlay();
	SetActorRotation(FRotationMatrix::MakeFromX(GetActorLocation() * -1.0).Rotator());
}

// Called every frame
void APlanetPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LastDeltaTime = DeltaTime;
}

// Called to bind functionality to input
void APlanetPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &APlanetPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveUp", this, &APlanetPawn::MoveUp);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlanetPawn::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APlanetPawn::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &APlanetPawn::LookUp);
	PlayerInputComponent->BindAction("AddMoveSpeed", IE_Pressed, this, &APlanetPawn::AddMoveSpeed);
	PlayerInputComponent->BindAction("SubMoveSpeed", IE_Pressed, this, &APlanetPawn::SubMoveSpeed);
}

void APlanetPawn::MoveRight(float Val)
{
	if (Val != 0.f)
	{
		FRotator Rotator = ArrowComponent->GetRelativeRotation();
		float Ag = ((Rotator.Roll - 60) / 180.0) * PI;
		float X = Val * MoveSpeed * cos(Ag)* LastDeltaTime;
		float Y = Val * MoveSpeed * sin(Ag)* LastDeltaTime;
		FVector Vector = {X, Y, SphereRadius };
		ModifyLocation(Vector);
	}
}

void APlanetPawn::MoveForward(float Val)
{
	if (Val != 0.f)
	{
		FRotator Rotator = ArrowComponent->GetRelativeRotation();
		float Ag = ((Rotator.Roll + 60) / 180.0) * PI;
		float X = -Val * MoveSpeed * cos(Ag)* LastDeltaTime;
		float Y = -Val * MoveSpeed * sin(Ag)* LastDeltaTime;
		FVector Vector = {X, Y, SphereRadius };
		ModifyLocation(Vector);
	}
}
void APlanetPawn::Turn(float Val)
{
	if (Val != 0.f)
	{
		ArrowComponent->AddRelativeRotation({0, 0, Val});
	}
}
void APlanetPawn::LookUp(float Val)
{
	if (Val != 0.f)
	{
		CameraComponent->AddRelativeRotation({-Val, 0, 0});
	}
}
void APlanetPawn::AddMoveSpeed()
{
	MoveSpeed = MoveSpeed * 2.0;
}

void APlanetPawn::SubMoveSpeed()
{
	MoveSpeed = MoveSpeed / 2.0;
}
void APlanetPawn::MoveUp(float Val)
{
	if (Val != 0.f)
	{
		FVector Vector = GetActorLocation();
		Vector.Normalize();
		FVector Position = Vector * Val * MoveSpeed * LastDeltaTime + GetActorLocation();
		SetActorLocation(Position);
	}
}
void APlanetPawn::ModifyLocation(const FVector &Pos, bool IsModifyHeight)
{
	FQuat NewQuat;

	FVector Vector = Pos;
	double Scale = Vector.Size() / SphereRadius;
	Vector.Normalize();
	FVectorDouble3D Location = GetActorLocation();
	Location.Normalize();

	FQuatDouble QuatD = UPlanetFunctionLib::GetQuatDouble({ 0,0,1 }, Location, Scale);

	FVectorDouble3D RealLocation = UPlanetFunctionLib::RotatorVector(Vector, QuatD);

	RealLocation.Normalize();

	FVectorDouble3D LD = RealLocation * GetActorLocation().Size();
	SetActorLocation(LD.ToVector());
}

void APlanetPawn::MovePoint(const FVector &Val)
{ 
}

void APlanetPawn::SetUsingAbsoluteRotation(USceneComponent *SceneComponent, bool bInAbsoluteRotation)
{
	SceneComponent->SetUsingAbsoluteRotation(bInAbsoluteRotation);
}
float APlanetPawn::GetPlayerHeight()
{
	return GetActorLocation().Size() - SphereRadius;
}

float APlanetPawn::GetPlayerMoveSpeed()
{
	return MoveSpeed;
}

void APlanetPawn::SetComponentRelativeRotation(UPrimitiveComponent *Src, UPrimitiveComponent *Dst)
{
	Dst->SetRelativeRotation_Direct(ArrowComponent->GetRelativeRotation());
	float Size = GetPlayerHeight() / 1000;
	FVector Location(-Size, 0, 0);
	Dst->SetRelativeLocation_Direct(Location);
}
#pragma optimize("", on)
