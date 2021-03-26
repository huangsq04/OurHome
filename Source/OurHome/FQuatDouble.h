// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RegularPentagonTriangleArea.h"

void SinCosA(double* ScalarSin, double* ScalarCos, double  Value);

/**
 * 
 */
class FQuatDouble
{
public:
	FQuatDouble();
	~FQuatDouble();

	/** The quaternion's X-component. */
	double X;

	/** The quaternion's Y-component. */
	double Y;

	/** The quaternion's Z-component. */
	double Z;

	/** The quaternion's W-component. */
	double W;

	FQuatDouble(double InX, double InY, double InZ, double InW)
	{

	}

	FQuatDouble(const FQuatDouble& Q)
	{
		
	}

	FQuatDouble(FVectorDouble3D Axis, double AngleRad);

	void Normalize(float Tolerance = SMALL_NUMBER)
	{
		const double SquareSum = X * X + Y * Y + Z * Z + W * W;

		if (SquareSum >= Tolerance)
		{
			const double Scale = FMath::InvSqrt(SquareSum);

			X *= Scale;
			Y *= Scale;
			Z *= Scale;
			W *= Scale;
		}
	}
	static FQuatDouble Slerp(const FQuatDouble& Quat1, const FQuatDouble& Quat2, double Slerp)
	{
		// Get cosine of angle between quats.
		const double RawCosom =
			Quat1.X * Quat2.X +
			Quat1.Y * Quat2.Y +
			Quat1.Z * Quat2.Z +
			Quat1.W * Quat2.W;
		// Unaligned quats - compensate, results in taking shorter route.
		const double Cosom = FMath::FloatSelect(RawCosom, RawCosom, -RawCosom);

		double Scale0, Scale1;

		if (Cosom < 0.9999f)
		{
			const double Omega = acos(Cosom);
			const double InvSin = 1.f / sin(Omega);
			Scale0 = sin((1.f - Slerp) * Omega) * InvSin;
			Scale1 = sin(Slerp * Omega) * InvSin;
		}
		else
		{
			// Use linear interpolation.
			Scale0 = 1.0f - Slerp;
			Scale1 = Slerp;
		}

		// In keeping with our flipped Cosom:
		Scale1 = FMath::FloatSelect(RawCosom, Scale1, -Scale1);

		FQuatDouble Result;

		Result.X = Scale0 * Quat1.X + Scale1 * Quat2.X;
		Result.Y = Scale0 * Quat1.Y + Scale1 * Quat2.Y;
		Result.Z = Scale0 * Quat1.Z + Scale1 * Quat2.Z;
		Result.W = Scale0 * Quat1.W + Scale1 * Quat2.W;
		Result.Normalize();
		return Result;
	}

	FVectorDouble3D RotateVector(FVectorDouble3D V) const
	{
		const FVectorDouble3D Q(X, Y, Z);
		const FVectorDouble3D T = (Q ^V) * 2.0;
		const FVectorDouble3D Result = V + (T * W) + (Q ^ T);
		return Result;
	}

};
