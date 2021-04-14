// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PlanetFunctionLib.generated.h"

class FVectorDouble2D;
class FVectorDouble3D;
class FQuatDouble;

FORCEINLINE float Sigmoid(float x)
{
	return (1 / (1 + exp(-x)));
}


/**
 * 
 */
UCLASS()
class PLANETPLUGIN_API UPlanetFunctionLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static uint64_t crc64(uint64_t crc, const unsigned char *s, uint64_t l);

	static uint64 GetTypeHash(const FVectorDouble2D& Vector);

	static uint64 GetTypeHash(const FVectorDouble3D& Vector);

	//Çò×ø±ê×ª»»µÑ¿¨¶û
	static FVectorDouble2D  SphericalFromCartesian(const FVectorDouble3D &InPoint);

	//µÑ¿¨¶û×ª»»Çò×ø±ê
	static FVectorDouble3D  SphericalToCartesian(const FVectorDouble2D &InPoint);

	static bool VectorsOnSameSide(const FVectorDouble3D& Vec, const FVectorDouble3D& A, 
		const FVectorDouble3D& B, const float SameSideDotProductEpsilon);

	static bool PointInTriangle(const FVectorDouble3D& A, const FVectorDouble3D& B, const FVectorDouble3D& C, 
		const FVectorDouble3D& P, const float InsideTriangleDotProductEpsilon = 0.000000000);

	static double PointToSegDist(const FVectorDouble3D &A, const FVectorDouble3D &B, const FVectorDouble3D &P);

	static FQuatDouble GetQuatDouble(const FVectorDouble3D &A, const FVectorDouble3D &B, double AngleRad);

	static FVectorDouble3D RotatorVector(const FVectorDouble3D &V, const FQuatDouble &Q);
};
