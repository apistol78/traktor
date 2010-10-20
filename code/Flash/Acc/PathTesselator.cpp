#include <algorithm>
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Flash/Path.h"
#include "Flash/Acc/PathTesselator.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

Vector2 evalQuadratic(
	float t,
	const Vector2& cp0,
	const Vector2& cp1,
	const Vector2& cp2
)
{
	float it = 1.0f - t;
	return (it * it) * cp0 + (2.0f * it * t) * cp1 + (t * t) * cp2;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.PathTesselator", PathTesselator, Object)

void PathTesselator::tesselate(const Path& path, AlignedVector< Segment >& outSegments) const
{
	const std::list< SubPath >& subPaths = path.getSubPaths();
	if (subPaths.empty())
		return;

	// Tesselate segments into linear segments.
	for (std::list< SubPath >::const_iterator i = subPaths.begin(); i != subPaths.end(); ++i)
	{
		for (std::vector< SubPathSegment >::const_iterator j = i->segments.begin(); j != i->segments.end(); ++j)
		{
			switch (j->type)
			{
			case SpgtLinear:
				tesselateLinearSegment(*i, *j, outSegments);
				break;

			case SpgtQuadratic:
				tesselateQuadraticSegment(*i, *j, outSegments);
				break;
			}
		}
	}
}

void PathTesselator::tesselateLinearSegment(const SubPath& subPath, const SubPathSegment& segment, AlignedVector< Segment >& outSegments) const
{
	T_ASSERT (segment.points.size() == 2);

	Segment s;
	s.v[0] = segment.points[0];
	s.v[1] = segment.points[1];
	s.fillStyle0 = subPath.fillStyle0;
	s.fillStyle1 = subPath.fillStyle1;
	s.lineStyle = subPath.lineStyle;
	outSegments.push_back(s);
}

void PathTesselator::tesselateQuadraticSegment(const SubPath& subPath, const SubPathSegment& segment, AlignedVector< Segment >& outSegments) const
{
	T_ASSERT (segment.points.size() == 3);

	const Vector2& cp0 = segment.points[0];
	const Vector2& cp1 = segment.points[1];
	const Vector2& cp2 = segment.points[2];

	Vector2 mn(std::numeric_limits< float >::max(), std::numeric_limits< float >::max());
	Vector2 mx(-std::numeric_limits< float >::max(), -std::numeric_limits< float >::max());
	for (std::vector< Vector2 >::const_iterator i = segment.points.begin(); i != segment.points.end(); ++i)
	{
		mn.x = min(mn.x, i->x);
		mn.y = min(mn.y, i->y);
		mx.x = max(mx.x, i->x);
		mx.y = max(mx.y, i->y);
	}

	Vector2 ps = cp0;
	Vector2 pm = evalQuadratic(0.5f, cp0, cp1, cp2);

	// Shape normalized coordinates.
	Vector2 extent = mx - mn;
	Vector2 nm1 = pm / extent;
	Vector2 nm2 = ((cp1 + cp0) * 0.5f) / extent;

	// Calculate number of steps.
	float distance = (nm1 - nm2).length();

	const float c_maxDistance = 0.7f;
	const float c_maxSteps = 12.0f;

	int steps = int(min(distance, c_maxDistance) * c_maxSteps / c_maxDistance);
	if (steps <= 0)
		steps = 1;
		
	outSegments.reserve(outSegments.size() + steps);

	for (int i = 0; i < steps; ++i)
	{
		float t = float(i + 1) / steps;
		Vector2 pe = evalQuadratic(t, cp0, cp1, cp2);

		Segment s;
		s.v[0] = ps;
		s.v[1] = pe;
		s.fillStyle0 = subPath.fillStyle0;
		s.fillStyle1 = subPath.fillStyle1;
		s.lineStyle = subPath.lineStyle;
		outSegments.push_back(s);

		ps = pe;
	}
}

	}
}
