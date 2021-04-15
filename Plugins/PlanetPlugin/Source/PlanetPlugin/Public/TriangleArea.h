// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FVectorDouble3D.h"
#include "FVectorDouble2D.h"
#include "PlanetFunctionLib.h"

/**
 * 
 */
class PLANETPLUGIN_API FTriangleArea
{
public:
	FTriangleArea();
	~FTriangleArea();

	FTriangleArea(const FVectorDouble3D &P1,
		const FVectorDouble3D &P2, const FVectorDouble3D &P3, int Idx);

	bool InArea(const FVectorDouble3D &Pos);
	bool BesideArea(const FVectorDouble3D &Pos, float Len);

	FVectorDouble3D LocalXYToWorldPosition(const FIntPoint &Point, int Level, bool UseRadius = false);
	FIntPoint GetCenter(const FVectorDouble3D &InPoint, const TArray<FIntPoint> &Idx, int Level, int MaxLevel);

	//获取点对应的6边形坐标
	FIntVector GetHexXY(const FVectorDouble3D& Point, int Level);

	//获取周边的6个6边形中心坐标
	TArray<FIntVector> GetHexRound(const FIntVector& Point);

	//获取周边的一定范围的6边形中心坐标
	TArray<FIntVector> GetHexRoundRange(const FIntVector& Point,uint8 Range = 1);

	//获取周边的6个顶点坐标
	TArray<FIntVector> GetHexPoint(const FIntVector& Point);

	FIntVector PointInTile(const FVectorDouble3D& Point, int Level);

	bool HasSide(const FVectorDouble3D& P0, const FVectorDouble3D& P1)
	{
		int H = 0;
		if (CenterPos == P0) H++;
		if (Pos0 == P0) H++;
		if (Pos1 == P0) H++;
		if (CenterPos == P1) H++;
		if (Pos0 == P1) H++;
		if (Pos1 == P1) H++;
		if (H == 2) return true;
		return false;
	}

	bool HasSide(const FVectorDouble3D& P0)
	{
		if (CenterPos == P0) return true;
		if (Pos0 == P0) return true;
		if (Pos1 == P0) return true;
		return false;
	}

	FVectorDouble3D Point2dToVector3d(const FVector2D &P, const FVectorDouble3D &P1,
		const FVectorDouble3D &P2, const FVectorDouble3D &P3, int Level);

	FVectorDouble3D PointFromLineSegment(const FVectorDouble3D &P1, const FVectorDouble3D &P2, int P, int Level);

	const FVectorDouble3D &GetCenterPos() { return CenterPos; };

	const FVectorDouble3D &GetPos0() { return Pos0; };

	const FVectorDouble3D &GetPos1() { return Pos1; };

	int GetIndex() { return Index; };

	static void CreateBaseDodecahedron(TArray<FTriangleArea> &OutArray);

	FVectorDouble3D CenterPos;
	FVectorDouble3D Pos0;
	FVectorDouble3D Pos1;

	FVectorDouble2D SphericalCoordinateCenter;
	FVectorDouble2D SphericalCoordinate0;
	FVectorDouble2D SphericalCoordinate1;

	double LenCentPos0;
	double LenCentPos1;
	double LenPos0Pos1;
	int Index;
	double MinAngle;
};
