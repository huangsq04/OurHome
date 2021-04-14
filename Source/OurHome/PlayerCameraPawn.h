// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlanetActor.h"
#include "PlayerCameraPawn.generated.h"

class USphereComponent;
class UCameraComponent;
class UArrowComponent;

UCLASS()
class OURHOME_API APlayerCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APlayerCameraPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void ModifyLocation(const FVector &Pos, bool IsModifyHeight = true);

	UFUNCTION(BlueprintCallable, Category = "PlayerCameraPawn")
	virtual void MoveForward(float Val);

	UFUNCTION(BlueprintCallable, Category = "PlayerCameraPawn")
	virtual void MoveRight(float Val);

	UFUNCTION(BlueprintCallable, Category = "PlayerCameraPawn")
	virtual void MoveUp(float Val);

	UFUNCTION(BlueprintCallable, Category = "PlayerCameraPawn")
	virtual void Turn(float Val);

	UFUNCTION(BlueprintCallable, Category = "PlayerCameraPawn")
	virtual void LookUp(float Val);

	UFUNCTION(BlueprintCallable, Category = "PlayerCameraPawn")
	void MovePoint(const FVector &Val);

	UFUNCTION(BlueprintCallable, Category = "PlayerCameraPawn")
	void SetUsingAbsoluteRotation(USceneComponent *SceneComponent, bool bInAbsoluteRotation);

	UFUNCTION(BlueprintCallable, Category = "PlayerCameraPawn")
	void ChangePlanetMovemount(APlanetActor *Planet);

	UFUNCTION(BlueprintCallable, Category = "PlayerCameraPawn")
	float GetPlayerHeight();

	UFUNCTION(BlueprintCallable, Category = "PlayerCameraPawn")
	float GetPlayerMoveSpeed();

private:
	void AddMoveSpeed();

	void SubMoveSpeed();

public:

	UFUNCTION(BlueprintCallable, Category = "PlayerCameraPawn")
	void SetPlanetActor(APlanetActor *Val) { PlanetActor = Val; };

	UFUNCTION(BlueprintCallable, Category = "PlayerCameraPawn")
	void SetComponentRelativeRotation(UPrimitiveComponent *Src, UPrimitiveComponent *Dst);

	UPROPERTY(Category = Pawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USphereComponent* CollisionComponent;

	UPROPERTY(Category = Pawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UArrowComponent* ArrowComponent;

	UPROPERTY(Category = Pawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UArrowComponent* ArrowComponent0;

	UPROPERTY(Category = Pawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* CameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerCameraPawn")
	float MoveSpeed = 500;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerCameraPawn")
	float MinAltitude = 100.0;

	UPROPERTY(Category = Pawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float Elevation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerCameraPawn")
	APlanetActor *PlanetActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerCameraPawn")
	float UpdateHeightMin = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerCameraPawn")
	float UpdateHeightMax = 600000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerCameraPawn")
	float PowParam = 0.015f;

private:

	FVector LastLocation;

	FVector TargetPoint;

	FVector LastUpdatePlanetPoint;
	float LastDeltaTime = 0.0;
};
