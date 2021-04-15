// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Planet.h"
#include "PlanetPawn.generated.h"

class USphereComponent;
class UCameraComponent;
class UArrowComponent;

UCLASS()
class PLANETPLUGIN_API APlanetPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APlanetPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void ModifyLocation(const FVector &Pos, bool IsModifyHeight = true);

	UFUNCTION(BlueprintCallable, Category = "PlanetPawn")
	virtual void MoveForward(float Val);

	UFUNCTION(BlueprintCallable, Category = "PlanetPawn")
	virtual void MoveRight(float Val);

	UFUNCTION(BlueprintCallable, Category = "PlanetPawn")
	virtual void MoveUp(float Val);

	UFUNCTION(BlueprintCallable, Category = "PlanetPawn")
	virtual void Turn(float Val);

	UFUNCTION(BlueprintCallable, Category = "PlanetPawn")
	virtual void LookUp(float Val);

	UFUNCTION(BlueprintCallable, Category = "PlanetPawn")
	void MovePoint(const FVector &Val);

	UFUNCTION(BlueprintCallable, Category = "PlanetPawn")
	void SetUsingAbsoluteRotation(USceneComponent *SceneComponent, bool bInAbsoluteRotation);

	UFUNCTION(BlueprintCallable, Category = "PlanetPawn")
	void ChangePlanet(APlanet *V) { Planet = V; };

	UFUNCTION(BlueprintCallable, Category = "PlanetPawn")
	float GetPlayerHeight();

	UFUNCTION(BlueprintCallable, Category = "PlanetPawn")
	float GetPlayerMoveSpeed();

	UFUNCTION(BlueprintCallable, Category = "PlanetPawn")
	void SetComponentRelativeRotation(UPrimitiveComponent *Src, UPrimitiveComponent *Dst);

private:
	void AddMoveSpeed();

	void SubMoveSpeed();

public:

	UPROPERTY(Category = Pawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USphereComponent* CollisionComponent;

	UPROPERTY(Category = Pawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UArrowComponent* ArrowComponent;

	UPROPERTY(Category = Pawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UArrowComponent* ArrowComponent0;

	UPROPERTY(Category = Pawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* CameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlanetPawn")
	float MoveSpeed = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlanetPawn")
	float SphereRadius = 600000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlanetPawn")
	APlanet *Planet;

private:

	float LastDeltaTime = 0.0;
};
