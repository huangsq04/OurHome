// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlanetFunctionLib.h"

/**
 * 
 */
class PLANETPLUGIN_API FVectorDouble2D
{
public:
	FVectorDouble2D() :X(0), Y(0) {}
	FVectorDouble2D(double InX, double InY) :X(InX), Y(InY) {}
	FVectorDouble2D(const FVector2D &V) :X(V.X), Y(V.Y) {}
	FVector2D ToVector() { return { (float)X,(float)Y }; }
	FVectorDouble2D operator+(const FVector2D& V) const
	{
		return FVectorDouble2D(X + V.X, Y + V.Y);
	}
	FVectorDouble2D operator+(const FVectorDouble2D& V) const
	{
		return FVectorDouble2D(X + V.X, Y + V.Y);
	}

	FVectorDouble2D operator-(const FVector2D& V) const
	{
		return FVectorDouble2D(X - V.X, Y - V.Y);
	}
	FVectorDouble2D operator-(const FVectorDouble2D& V) const
	{
		return FVectorDouble2D(X - V.X, Y - V.Y);
	}
	FVectorDouble2D operator*(float Scale) const
	{
		return FVectorDouble2D(X * Scale, Y * Scale);
	}

	bool operator==(const FVectorDouble2D& V) const
	{
		return X == V.X && Y == V.Y;
	}

	bool operator!=(const FVectorDouble2D& V) const
	{
		return X != V.X || Y != V.Y;
	}
	void Normalize()
	{
		const double SquareSum = X * X + Y * Y;
		if (SquareSum > 0)
		{
			const float Scale = 1.0f / sqrtf(SquareSum);
			X *= Scale; Y *= Scale;
		}
	}
	FVector2D ToVector2D() { return {(float)X, (float)Y}; }

	double X;
	double Y;
};
