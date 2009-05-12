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

Plane Winding::plane() const
{
	T_ASSERT (points.size() >= 3);
	
	size_t i = 2;
	for (; i < points.size(); ++i)
	{
		Vector4 e1 = (points[1] - points[0]).normalized();
		Vector4 e2 = (points[i] - points[0]).normalized();
		if (std::fabs(1.0f - dot3(e1, e2)) > FUZZY_EPSILON)
			break;
	}
	T_ASSERT (i < points.size());

	return Plane(points[0], points[1], points[i]);
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

		if (da >= FUZZY_EPSILON)
			outFront.points.push_back(a);
		else
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
