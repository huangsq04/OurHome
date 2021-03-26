// Fill out your copyright notice in the Description page of Project Settings.


#include "RegularPentagonTriangleArea.h"
#include "PlanetActor.h"
#include "Engine/Public/GeomTools.h"

#pragma optimize("", off)

bool VectorsOnSameSide(const FVectorDouble3D& Vec, const FVectorDouble3D& A, 
	const FVectorDouble3D& B, const float SameSideDotProductEpsilon)
{
	FVectorDouble3D AB = A - B;
	FVectorDouble3D PB = Vec - B;
	AB.Normalize();
	PB.Normalize();
	FVectorDouble3D CrossA = AB ^ PB;
	CrossA.Normalize();
	double DotWithEpsilon = SameSideDotProductEpsilon + (CrossA | Vec);
	return !FMath::IsNegativeDouble(DotWithEpsilon);
}
bool PointInTriangle(const FVectorDouble3D& A, const FVectorDouble3D& B, 
	const FVectorDouble3D& C, const FVectorDouble3D& P, const float InsideTriangleDotProductEpsilon)
{
	if (VectorsOnSameSide(A, B, P, InsideTriangleDotProductEpsilon) &&
		VectorsOnSameSide(B, C, P, InsideTriangleDotProductEpsilon) &&
		VectorsOnSameSide(C, A, P, InsideTriangleDotProductEpsilon))
	{
		return true;
	}
	else
	{
		return false;
	}
}
double PointToSegDist(const FVectorDouble3D &A, const FVectorDouble3D &B, const FVectorDouble3D &P)
{
	FVectorDouble3D AP = P - A;
	AP.Normalize();
	FVectorDouble3D AB = B - A;
	AB.Normalize();
	FVectorDouble3D BP = P - B;
	BP.Normalize();
	FVectorDouble3D BA = A - B;
	BA.Normalize();
	double AnglePAB = acos(FMath::Clamp(AP|AB, (double)-1.0, (double)1.0));
	double AnglePBA = acos(FMath::Clamp(BP|BA, (double)-1.0, (double)1.0));
	if (AnglePAB > PI / 2.0 || AnglePBA > PI / 2.0)
	{
		float DisPA = (P - A).Size();
		float DisPB = (P - B).Size();
		if (DisPA > DisPB)
		{
			return DisPB;
		}

		return DisPA;
	}
	else
	{
		float DisPA = (P - A).Size();
		return sin(AnglePAB) * DisPA;

	}
	return 0.0;
}
void FVectorDouble3D::Normalize(float Tolerance)
{
	const double SquareSum = X * X + Y * Y + Z * Z;
	if (SquareSum > Tolerance)
	{
		const double Scale = 1.0f / sqrt(SquareSum);
		X *= Scale; Y *= Scale; Z *= Scale;
	}
}
void FRegularPentagonTriangleArea::CreateBaseDodecahedron(TArray<FRegularPentagonTriangleArea> &OutArray)
{
	if (OutArray.Num() != 0)
	{
		return;
	}

	double th = 1.618033988749895;
	double ath = 0.6180339887498948;
	TArray<int32> BaseTriangles;
	// 构造正5面体 
	TArray<FVectorDouble3D> BaseVertices = {
	{ 1.0, 1.0, 1.0}, { 1.0, 1.0, -1.0}, { 1.0, -1.0, -1.0},
	{ -1.0, -1.0, -1.0}, { -1.0, -1.0, 1.0}, { -1.0, 1.0, 1.0},
	{ -1.0, 1.0, -1.0}, { 1.0, -1.0, 1},
	{0, th, ath},{0, -th, ath},{0, th, -ath},{0, -th, -ath},
	{ath, 0, th},{-ath, 0, th},{ath, 0, -th},{-ath, 0, -th},
	{th, ath, 0},{-th, ath, 0},{th, -ath, 0},{-th, -ath, 0}
	};

	for (int i = 0; i < BaseVertices.Num(); i++)
	{
		BaseVertices[i].Normalize();
		BaseVertices[i] = BaseVertices[i].RotateAngleAxis(-31.71768, FVector(1, 0, 0));
		BaseVertices[i] = BaseVertices[i].RotateAngleAxis(90, FVector(0, 0, 1));
	}
	TArray<int32> RegularPentagon = {
		12, 13, 4,  9,  7,
		13, 12, 0,  8,  5,
		0,  12, 7,  18, 16,
		4,  13, 5,  17, 19,
		8,  0,  16, 1,  10,
		9,  4,  19, 3,  11,
		1,  16, 18, 2,  14,
		17, 5,  8,  10, 6,
		18, 7,  9,  11, 2,
		6,  10, 1,  14, 15,
		3,  19, 17, 6,  15,
		2,  11, 3,  15, 14,
	};
	for (int i = 0; i < RegularPentagon.Num() / 5; i++)
	{
		int Ix = i * 5;

		FVectorDouble3D CN1 = BaseVertices[RegularPentagon[Ix + 0]] - BaseVertices[RegularPentagon[Ix + 1]];
		float CL1 = CN1.Size();
		CN1.Normalize();
		FVectorDouble3D CN2 = BaseVertices[RegularPentagon[Ix + 0]] - CN1 * CL1 / 2.0;

		FVectorDouble3D CN3 = CN2 - BaseVertices[RegularPentagon[Ix + 3]];
		float CL3 = CN3.Size();
		CN3.Normalize();
		FVectorDouble3D C4 = CN2 - CN3 * CL3 * 0.447213595;

		C4.Normalize();
		int Idx = BaseVertices.Num();
		BaseVertices.Add(C4);

		BaseTriangles.Add(RegularPentagon[Ix + 1]);
		BaseTriangles.Add(Idx);
		BaseTriangles.Add(RegularPentagon[Ix + 0]);

		BaseTriangles.Add(RegularPentagon[Ix + 2]);
		BaseTriangles.Add(Idx);
		BaseTriangles.Add(RegularPentagon[Ix + 1]);

		BaseTriangles.Add(RegularPentagon[Ix + 3]);
		BaseTriangles.Add(Idx);
		BaseTriangles.Add(RegularPentagon[Ix + 2]);

		BaseTriangles.Add(RegularPentagon[Ix + 4]);
		BaseTriangles.Add(Idx);
		BaseTriangles.Add(RegularPentagon[Ix + 3]);

		BaseTriangles.Add(RegularPentagon[Ix + 0]);
		BaseTriangles.Add(Idx);
		BaseTriangles.Add(RegularPentagon[Ix + 4]);

		OutArray.Add(FRegularPentagonTriangleArea(C4, BaseVertices[RegularPentagon[Ix + 1]],
			BaseVertices[RegularPentagon[Ix + 0]], OutArray.Num()));
		OutArray.Add(FRegularPentagonTriangleArea(C4, BaseVertices[RegularPentagon[Ix + 2]],
			BaseVertices[RegularPentagon[Ix + 1]], OutArray.Num()));
		OutArray.Add(FRegularPentagonTriangleArea(C4, BaseVertices[RegularPentagon[Ix + 3]],
			BaseVertices[RegularPentagon[Ix + 2]], OutArray.Num()));
		OutArray.Add(FRegularPentagonTriangleArea(C4, BaseVertices[RegularPentagon[Ix + 4]],
			BaseVertices[RegularPentagon[Ix + 3]], OutArray.Num()));
		OutArray.Add(FRegularPentagonTriangleArea(C4, BaseVertices[RegularPentagon[Ix + 0]],
			BaseVertices[RegularPentagon[Ix + 4]], OutArray.Num()));
	}

}
FRegularPentagonTriangleArea::FRegularPentagonTriangleArea()
{
}
FRegularPentagonTriangleArea::FRegularPentagonTriangleArea(const FVectorDouble3D &P1, 
	const FVectorDouble3D &P2, const FVectorDouble3D &P3, int Idx):
	CenterPos(P1), Pos0(P2), Pos1(P3), Index(Idx)
{
	LenCentPos0 = (CenterPos - Pos0).SizeSquared();
	LenCentPos1 = (CenterPos - Pos1).SizeSquared();
	LenPos0Pos1 = (Pos1 - Pos0).SizeSquared();

	SphericalCoordinateCenter = SphericalFromCartesian(P1);
	SphericalCoordinate0 = SphericalFromCartesian(P2);
	SphericalCoordinate1 = SphericalFromCartesian(P3);

	MinAngle = acos(Pos0|Pos1);
}

FRegularPentagonTriangleArea::~FRegularPentagonTriangleArea()
{
}
bool FRegularPentagonTriangleArea::BesideArea(const FVectorDouble3D &Pos, float Len)
{
	double DistCent = (CenterPos - Pos).SizeSquared();
	double DistPos0 = (Pos0 - Pos).SizeSquared();
	double DistPos1 = (Pos1 - Pos).SizeSquared();

	if (LenCentPos0 * 0.9 > DistCent + DistPos0) return true;
	if (LenCentPos1 * 0.9 > DistCent + DistPos1) return true;
	if (LenPos0Pos1 * 0.9 > DistPos1 + DistPos0) return true;
	return false;
}

bool FRegularPentagonTriangleArea::InArea(const FVectorDouble3D &Pos)
{
	return PointInTriangle(CenterPos, Pos0, Pos1, Pos);
}

FVectorDouble3D FRegularPentagonTriangleArea::LocalXYToWorldPosition(const FIntPoint &Point, int Level, bool UseRadius)
{
	int Layer = FMath::Pow(2, Level);
	float Scale = 1;

	return Point2dToVector3d(Point, Pos1, CenterPos, Pos0, Layer);
}

FIntPoint FRegularPentagonTriangleArea::GetCenter(const FVectorDouble3D &InPoint, const TArray<FIntPoint> &Idx, int Level, int MaxLevel)
{
	int Layer = FMath::Pow(2, Level);
	int MaxLayer = FMath::Pow(2, MaxLevel);
	TArray<int32> InTriangles = {
	0,  1,  2,
	1,  3,  4,
	1,  4,  2,
	2,  4,  5,
	};
	TArray<FIntPoint> ITP;
	FIntPoint CD1 = Idx[0] + Idx[1];
	FIntPoint CD2 = Idx[0] + Idx[2];
	FIntPoint CD3 = Idx[1] + Idx[2];
	ITP.Add(Idx[0] * 2);
	ITP.Add(CD1);
	ITP.Add(CD2);
	ITP.Add(Idx[1] * 2);
	ITP.Add(CD3);
	ITP.Add(Idx[2] * 2);

	for (int i = 0; i < InTriangles.Num() / 3; i++)
	{
		int Ix = i * 3;
		FVectorDouble3D VT10 = LocalXYToWorldPosition(ITP[InTriangles[Ix]], Level, false);
		FVectorDouble3D VT11 = LocalXYToWorldPosition(ITP[InTriangles[Ix + 1]], Level, false);
		FVectorDouble3D VT12 = LocalXYToWorldPosition(ITP[InTriangles[Ix + 2]], Level, false);
		bool In = PointInTriangle(VT10, VT11, VT12, InPoint);

		if (In)
		{
			TArray<FIntPoint> T;
			T.Add(ITP[InTriangles[Ix]]);
			T.Add(ITP[InTriangles[Ix + 1]]);
			T.Add(ITP[InTriangles[Ix + 2]]);

			if (Level == MaxLevel)
			{
				TArray<FIntPoint> R;
				if ((T[0].X - T[0].Y) % 3 == 0) R.Add(T[0]);
				if ((T[1].X - T[1].Y) % 3 == 0) R.Add(T[1]);
				if ((T[2].X - T[2].Y) % 3 == 0) R.Add(T[2]);
				if (R.Num() == 1)
				{
					return R[0];
				}
				return  FIntPoint(-2, -2);
			}
			FIntPoint Ret = GetCenter(InPoint, T, Level + 1, MaxLevel);

			if (Ret.X == -1)
			{
				FVectorDouble3D M = InPoint + ((VT10 + VT11 + VT12) / 3.0 - InPoint) / MaxLevel;
				M.Normalize();
				UE_LOG(LogTemp, Warning, TEXT("Modify point ..!"));
				return GetCenter(M, T, Level + 1, MaxLevel);
			}
			else
			{
				return Ret;
			}
		}
	}
	return  FIntPoint(-1, -1);
}

FIntVector FRegularPentagonTriangleArea::GetHexXY(const FVectorDouble3D& Point, int Level)
{
	float Angle = acos(Point|CenterPos);
	if (Angle > 1.5)
	{
		return FIntVector(-1, -1, -1);
	}

	if (InArea(Point))
	{
		TArray<FIntPoint> Idx;
		Idx.Add(FIntPoint(0, 0));
		Idx.Add(FIntPoint(1, 0));
		Idx.Add(FIntPoint(0, 1));
		FIntPoint P = GetCenter(Point, Idx, 1, Level);
		if (P.X == -1)
		{
			check(0);
		}
		return FIntVector(P.X, P.Y, Index);
	}
	return FIntVector(-1, -1, -1);
}
FIntVector FRegularPentagonTriangleArea::PointInTile(const FVectorDouble3D& Point, int Level)
{
	FVectorDouble3D P = Point;
	P.Normalize();
	if (InArea(P))
	{
		TArray<FIntPoint> Idx;
		Idx.Add(FIntPoint(0, 0));
		Idx.Add(FIntPoint(1, 0));
		Idx.Add(FIntPoint(0, 1));
		FIntPoint RP = GetCenter(P, Idx, 1, Level);
		if (RP.X == -1)
		{
			check(0);
		}
		return FIntVector(RP.X, RP.Y, Index);
	}
	return FIntVector(-1, -1, -1);
}
FVectorDouble3D FRegularPentagonTriangleArea::Point2dToVector3d(const FVector2D &P, const FVectorDouble3D &P1, 
	const FVectorDouble3D &P2, const FVectorDouble3D &P3, int Level)
{
	if (P.X < 0 || P.Y < 0)
	{
		check(0);
	}
	if (P.X == 0 && P.Y == 0)
	{
		return P2;
	}
	else if (P.X == 0)
	{
		if (P.Y == Level)
		{
			return P1;
		}

		return PointFromLineSegment(P2, P1, P.Y, Level);
	}
	else if (P.Y == 0)
	{
		if (P.X == Level)
		{
			return P3;
		}
		return PointFromLineSegment(P2, P3, P.X, Level);
	}
	else if (P.Y + P.X == Level)
	{
		return PointFromLineSegment(P1, P3, P.X, Level);
	}
	else if (P.Y + P.X < Level)
	{
		FVectorDouble3D CentPoint[3] = { (P2 + P1) * 0.5, (P1 + P3) * 0.5, (P2 + P3) * 0.5 };
		CentPoint[0].Normalize();
		CentPoint[1].Normalize();
		CentPoint[2].Normalize();
		if (P.X < Level / 2 && P.Y < Level / 2)
		{
			if (P.X + P.Y <= Level / 2)
			{
				return Point2dToVector3d(P, CentPoint[0], P2, CentPoint[2], Level / 2);
			}
			else
			{
				FVector2D  MP(abs(P.Y - Level / 2), abs(P.X - Level / 2));
				return Point2dToVector3d(MP, CentPoint[0], CentPoint[1], CentPoint[2], Level / 2);
			}
		}
		else if (P.X >= Level / 2)
		{
			FVector2D  MP(P.X - Level / 2, P.Y);
			return Point2dToVector3d(MP, CentPoint[1], CentPoint[2], P3, Level / 2);
		}
		else if (P.Y >= Level / 2)
		{
			FVector2D  MP(P.X, P.Y - Level / 2);
			return Point2dToVector3d(MP, P1, CentPoint[0], CentPoint[1], Level / 2);
		}
	}

	return { 0, 0, 0 };
}

FVectorDouble3D FRegularPentagonTriangleArea::PointFromLineSegment(const FVectorDouble3D &P1, 
	const FVectorDouble3D &P2, int P, int Level)
{
	if (Level == 2)
	{
		if (P == 0) return P1;
		if (P == 2) return P2;
		if (P == 1)
		{
			FVectorDouble3D R = (P2 + P1) * 0.5;
			R.Normalize();
			return R;
		}
		return{ 0,0,0 };
	}

	FVectorDouble3D  C0 = (P2 + P1) * 0.5;
	C0.Normalize();

	if (P < Level / 2)
	{
		return PointFromLineSegment(P1, C0, P, Level / 2);
	}
	else
	{
		return PointFromLineSegment(C0, P2, P - Level / 2, Level / 2);
	}
}
#pragma optimize("", on)
