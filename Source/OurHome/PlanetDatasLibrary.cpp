// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanetDatasLibrary.h"
#include "RegularPentagonTriangleArea.h"

TArray<int> UPlanetDatasLibrary::GetHexObject(FIntVector Vector)
{
	TArray<int> R;
	if (Vector.X == 1186 && Vector.Y == 3883 && Vector.Z == 44)
	{
		//R.Add(0);
	}
	return R;
}
float UPlanetDatasLibrary::TempPointToSegDist(const FVector &A, const FVector &B, const FVector &P) //Temp
{
	return PointToSegDist(A, B, P);
}
