// Fill out your copyright notice in the Description page of Project Settings.


#include "FQuatDouble.h"

#pragma optimize("", off)
#define D_PI 					(3.1415926535897932384626433832795f)
void SinCosA(double* ScalarSin, double* ScalarCos, double  Value)
{
	// Map Value to y in [-pi,pi], x = 2*pi*quotient + remainder.
	double quotient = (INV_PI*0.5f)*Value;
	if (Value >= 0.0f)
	{
		quotient = (double)((int)(quotient + 0.5f));
	}
	else
	{
		quotient = (double)((int)(quotient - 0.5f));
	}
	double y = Value - (2.0f*D_PI)*quotient;

	// Map y to [-pi/2,pi/2] with sin(y) = sin(Value).
	double sign;
	if (y > HALF_PI)
	{
		y = D_PI - y;
		sign = -1.0f;
	}
	else if (y < -HALF_PI)
	{
		y = -D_PI - y;
		sign = -1.0f;
	}
	else
	{
		sign = +1.0f;
	}

	double y2 = y * y;

	// 11-degree minimax approximation
	*ScalarSin = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;

	// 10-degree minimax approximation
	double p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
	*ScalarCos = sign * p;
}

FQuatDouble::FQuatDouble()
{
}

FQuatDouble::~FQuatDouble()
{
}
FQuatDouble::FQuatDouble(const FVectorDouble3D &Axis, double AngleRad)
{
	const double half_a = 0.5f * AngleRad;
	double s, c;
	SinCosA(&s, &c, half_a);

	X = s * Axis.X;
	Y = s * Axis.Y;
	Z = s * Axis.Z;
	W = c;
}
#pragma optimize("", on)
