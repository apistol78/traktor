/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Sphere.h"

namespace traktor
{

Sphere::Sphere(const Vector4& center_, const Scalar& radius_)
:	center(center_)
,	radius(radius_)
{
}

bool Sphere::inside(const Vector4& pt) const
{
	Vector4 d = pt - center;
	return dot3(d, d) <= radius * radius;
}

bool Sphere::intersectRay(const Vector4& p, const Vector4& d, Scalar& outDistance) const
{
	Vector4 oc = p - center;
	Scalar a = dot3(d, d);
	Scalar b = 2.0_simd * dot3(oc, d);
	Scalar c = dot3(oc, oc) - radius * radius;
	Scalar discriminant = b * b - 4.0_simd * a * c;
	if (discriminant >= 0.0_simd)
	{
		outDistance = (-b - squareRoot(discriminant)) / (2.0_simd * a);
		return true;
	}
	else
		return false;
}

bool Sphere::intersectRay(const Vector4& p, const Vector4& d, Scalar& outDistanceEnter, Scalar& outDistanceExit) const
{
	Vector4 oc = p - center;
	Scalar a = dot3(d, d);
	Scalar b = 2.0_simd * dot3(oc, d);
	Scalar c = dot3(oc, oc) - radius * radius;
	Scalar discriminant = b * b - 4.0_simd * a * c;
	if (discriminant >= 0.0_simd)
	{
		Scalar sqrtd = squareRoot(discriminant);
		outDistanceEnter = (-b - sqrtd) / (2.0_simd * a);
		outDistanceExit = (-b + sqrtd) / (2.0_simd * a);
		return true;
	}
	else
		return false;
}

}
