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

Vector2i evalQuadratic(
	float t,
	const Vector2i& cp0,
	const Vector2i& cp1,
	const Vector2i& cp2
)
{
	float it = 1.0f - t;
	Vector2 p = (it * it) * cp0.toVector2() + (2.0f * it * t) * cp1.toVector2() + (t * t) * cp2.toVector2();
	return Vector2i::fromVector2(p);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.PathTesselator", PathTesselator, Object)

void PathTesselator::tesselate(const Path& path, AlignedVector< Segment >& outSegments) const
{
	const std::list< SubPath >& subPaths = path.getSubPaths();
	if (subPaths.empty())
		return;

	outSegments.reserve(subPaths.size() * 10);

	// Tessellate segments into linear segments.
	for (std::list< SubPath >::const_iterator i = subPaths.begin(); i != subPaths.end(); ++i)
	{
		for (std::vector< SubPathSegment >::const_iterator j = i->segments.begin(); j != i->segments.end(); ++j)
		{
			switch (j->type)
			{
			case SpgtLinear:
				tesselateLinearSegment(path, *i, *j, outSegments);
				break;

			case SpgtQuadratic:
				tesselateQuadraticSegment(path, *i, *j, outSegments);
				break;
				
			default:
				break;
			}
		}
	}
}

void PathTesselator::tesselateLinearSegment(const Path& path, const SubPath& subPath, const SubPathSegment& segment, AlignedVector< Segment >& outSegments) const
{
	T_ASSERT (segment.pointsCount == 2);

	Segment s;
	s.v[0] = path.getPoints().at(segment.pointsOffset);
	s.v[1] = path.getPoints().at(segment.pointsOffset + 1);
	s.fillStyle0 = subPath.fillStyle0;
	s.fillStyle1 = subPath.fillStyle1;
	s.lineStyle = subPath.lineStyle;
	outSegments.push_back(s);
}

void PathTesselator::tesselateQuadraticSegment(const Path& path, const SubPath& subPath, const SubPathSegment& segment, AlignedVector< Segment >& outSegments) const
{
	T_ASSERT (segment.pointsCount == 3);

	const Vector2i& cp0 = path.getPoints().at(segment.pointsOffset);
	const Vector2i& cp1 = path.getPoints().at(segment.pointsOffset + 1);
	const Vector2i& cp2 = path.getPoints().at(segment.pointsOffset + 2);

	Vector2i mn(std::numeric_limits< int32_t >::max(), std::numeric_limits< int32_t >::max());
	Vector2i mx(-std::numeric_limits< int32_t >::max(), -std::numeric_limits< int32_t >::max());
	for (uint32_t i = 0; i < segment.pointsCount; ++i)
	{
		const Vector2i& cp = path.getPoints().at(segment.pointsOffset + i);
		mn.x = min(mn.x, cp.x);
		mn.y = min(mn.y, cp.y);
		mx.x = max(mx.x, cp.x);
		mx.y = max(mx.y, cp.y);
	}

	// Shape normalized coordinates.
	Vector2 pm = evalQuadratic(0.5f, cp0, cp1, cp2).toVector2();
	Vector2 extent = (mx - mn).toVector2();
	Vector2 nm1 = pm / extent;
	Vector2 nm2 = ((cp1 + cp0).toVector2() * 0.5f) / extent;

	// Calculate number of steps.
	float distance = (nm1 - nm2).length();

	const float c_maxDistance = 0.7f;
	const float c_maxSteps = 8.0f;

	int steps = int(min(distance, c_maxDistance) * c_maxSteps / c_maxDistance);
	if (steps <= 0)
		steps = 1;

	Vector2i ps = cp0;
	for (int i = 0; i < steps; ++i)
	{
		float t = float(i + 1) / steps;
		Vector2i pe = evalQuadratic(t, cp0, cp1, cp2);

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
