// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PlanetDatasLibrary.generated.h"

/**
 * 
 */
UCLASS()
class OURHOME_API UPlanetDatasLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable)
	static TArray<int> GetHexObject(FIntVector Vector);

	UFUNCTION(BlueprintCallable)
		static float TempPointToSegDist(const FVector &A, const FVector &B, const FVector &P); //Temp
};
