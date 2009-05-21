#include "Core/Math/Winding.h"
#include "Core/Math/Float.h"
#include "Core/Math/Const.h"

namespace traktor
{

Winding::Winding()
{
}

Winding::Winding(const AlignedVector< Vector4 >& points_)
:	points(points_)
{
}

Winding::Winding(const Vector4* points_, size_t npoints)
:	points(npoints)
{
	for (size_t i = 0; i < npoints; ++i)
		points[i] = points_[i];
}

bool Winding::angleIndices(uint32_t& outI1, uint32_t& outI2, uint32_t& outI3) const
{
	if (points.size() < 3)
		return false;

	for (size_t p = points.size() - 1, i = 0; i < points.size(); p = i++)
	{
		size_t n = (i + 1) % points.size();

		const Vector4& p1 = points[p];
		const Vector4& p2 = points[i];
		const Vector4& p3 = points[n];

		Vector4 e1 = p1 - p2;
		Vector4 e2 = p3 - p2;

		Scalar ln1 = e1.length();
		Scalar ln2 = e2.length();

		if (ln1 > FUZZY_EPSILON && ln2 > FUZZY_EPSILON)
		{
			e1 /= ln1;
			e2 /= ln2;

			Scalar phi = dot3(e1, e2);
			if (abs(phi) < 1.0f - FUZZY_EPSILON)
			{
				outI1 = p;
				outI2 = i;
				outI3 = n;
				return true;
			}
		}
	}

	return false;
}

bool Winding::getPlane(Plane& outPlane) const
{
	// Get indices to points which form a good frame.
	uint32_t i1, i2, i3;
	if (!angleIndices(i1, i2, i3))
		return false;

	// Create plane from indexed points.
	outPlane = Plane(points[i1], points[i2], points[i3]);
	return true;
}

void Winding::split(const Plane& plane, Winding& outFront, Winding& outBack) const
{
	for (size_t i = 0, j = points.size() - 1; i < points.size(); j = i++)
	{
		const Vector4& a = points[i];
		const Vector4& b = points[j];

		Scalar da = plane.distance(a);
		Scalar db = plane.distance(b);

		if ((da < -FUZZY_EPSILON && db > FUZZY_EPSILON) || (da > FUZZY_EPSILON && db < -FUZZY_EPSILON))
		{
			Scalar k;
			plane.segmentIntersection(a, b, k);
			T_ASSERT (k >= 0.0f && k <= 1.0f);

			Vector4 p = lerp(a, b, k);
			outFront.points.push_back(p);
			outBack.points.push_back(p);
		}

		if (da >= -FUZZY_EPSILON)
			outFront.points.push_back(a);
		if (da <= FUZZY_EPSILON)
			outBack.points.push_back(a);
	}
}

int Winding::classify(const Plane& plane) const
{
	int side[2] = { 0, 0 };
	for (size_t i = 0; i < points.size(); ++i)
	{
		float d = plane.distance(points[i]);
		if (d > FUZZY_EPSILON)
			side[CfFront]++;
		else if (d < -FUZZY_EPSILON)
			side[CfBack]++;
	}

	if (side[CfFront] && !side[CfBack])
		return CfFront;
	else if (!side[CfFront] && side[CfBack])
		return CfBack;
	else if (!side[CfFront] && !side[CfBack])
		return CfCoplanar;

	T_ASSERT (side[CfFront] && side[CfBack]);
	return CfSpan;
}

}
