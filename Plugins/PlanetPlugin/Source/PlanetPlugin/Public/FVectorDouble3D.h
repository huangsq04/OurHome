// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlanetFunctionLib.h"

/**
 * 
 */
class PLANETPLUGIN_API FVectorDouble3D
{
public:
	FVectorDouble3D() :X(0), Y(0), Z(0) {}
	FVectorDouble3D(double InX, double InY, double InZ) :X(InX), Y(InY), Z(InZ) {}
	FVectorDouble3D(const FVector &V) :X(V.X), Y(V.Y), Z(V.Z) {}
	FVectorDouble3D operator^(const FVectorDouble3D& V) const
	{
		return FVectorDouble3D
		(
			Y * V.Z - Z * V.Y,
			Z * V.X - X * V.Z,
			X * V.Y - Y * V.X
		);
	}
	double operator|(const FVectorDouble3D& V) const
	{
		return X * V.X + Y * V.Y + Z * V.Z;
	}
	FVectorDouble3D operator+(const FVector& V) const
	{
		return FVectorDouble3D(X + V.X, Y + V.Y, Z + V.Z);
	}
	FVectorDouble3D operator+(const FVectorDouble3D& V) const
	{
		return FVectorDouble3D(X + V.X, Y + V.Y, Z + V.Z);
	}

	FVectorDouble3D operator-(const FVector& V) const
	{
		return FVectorDouble3D(X - V.X, Y - V.Y, Z - V.Z);
	}
	FVectorDouble3D operator-(const FVectorDouble3D& V) const
	{
		return FVectorDouble3D(X - V.X, Y - V.Y, Z - V.Z);
	}
	FVectorDouble3D operator*(double Scale) const
	{
		return FVectorDouble3D(X * Scale, Y * Scale, Z * Scale);
	}
	FVectorDouble3D operator/(double Scale) const
	{
		return FVectorDouble3D(X / Scale, Y / Scale, Z / Scale);
	}

	bool operator==(const FVectorDouble3D& V) const
	{
		return X == V.X && Y == V.Y && Z == V.Z;
	}

	bool operator!=(const FVectorDouble3D& V) const
	{
		return X != V.X || Y != V.Y || Z != V.Z;
	}

	FVector ToVector() { return FVector( (float)X,(float)Y,(float)Z ); }

	void Normalize(float Tolerance = SMALL_NUMBER)
	{
		const double SquareSum = X * X + Y * Y + Z * Z;
		if (SquareSum > Tolerance)
		{
			const double Scale = 1.0f / sqrt(SquareSum);
			X *= Scale; Y *= Scale; Z *= Scale;
		}
	}
	float Size() const
	{
		return sqrt(X*X + Y*Y + Z*Z);
	}

	double SizeSquared() const
	{
		return X*X + Y*Y + Z*Z;
	}

	inline FVectorDouble3D RotateAngleAxis(const float AngleDeg, const FVectorDouble3D& Axis) const
	{
		float S, C;
		FMath::SinCos(&S, &C, FMath::DegreesToRadians(AngleDeg));

		const float XX = Axis.X * Axis.X;
		const float YY = Axis.Y * Axis.Y;
		const float ZZ = Axis.Z * Axis.Z;

		const float XY = Axis.X * Axis.Y;
		const float YZ = Axis.Y * Axis.Z;
		const float ZX = Axis.Z * Axis.X;

		const float XS = Axis.X * S;
		const float YS = Axis.Y * S;
		const float ZS = Axis.Z * S;

		const float OMC = 1.f - C;

		return FVectorDouble3D(
			(OMC * XX + C) * X + (OMC * XY - ZS) * Y + (OMC * ZX + YS) * Z,
			(OMC * XY + ZS) * X + (OMC * YY + C) * Y + (OMC * YZ - XS) * Z,
			(OMC * ZX - YS) * X + (OMC * YZ + XS) * Y + (OMC * ZZ + C) * Z
		);
	}

	double X;
	double Y;
	double Z;
};
