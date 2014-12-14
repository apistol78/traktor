#include <limits>
#include "Core/Math/Aabb2.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"

namespace traktor
{
	namespace
	{

bool intersectSegment1D(float smin, float smax, float raystart, float rayend, float& outEnter, float& outExit)
{
	float raydir = rayend - raystart;

	if (traktor::abs< float >(raydir) <= FUZZY_EPSILON)
	{
		if (raystart < smin || raystart > smax)
			return false;
		else
			return true;
	}

	float enter = (smin - raystart) / raydir;
	float exit = (smax - raystart) / raydir;

	if (enter > exit)
		std::swap< float >(enter, exit);

	if (outEnter > exit || enter > outExit)
		return false;

	outEnter = max< float >(outEnter, enter);
	outExit = min< float >(outExit, exit);
	return true;
}

	}

Aabb2::Aabb2()
:	mn(std::numeric_limits< float >::max(), std::numeric_limits< float >::max())
,	mx(-std::numeric_limits< float >::max(), -std::numeric_limits< float >::max())
{
}

Aabb2::Aabb2(const Aabb2& aabb)
:	mn(aabb.mn)
,	mx(aabb.mx)
{
}

Aabb2::Aabb2(const Vector2& mn_, const Vector2& mx_)
:	mn(mn_)
,	mx(mx_)
{
}

void Aabb2::getExtents(Vector2 extents[4]) const
{
	extents[0] = Vector2(mn.x, mn.y);
	extents[1] = Vector2(mx.x, mn.y);
	extents[2] = Vector2(mx.x, mx.y);
	extents[3] = Vector2(mn.x, mx.y);
}

bool Aabb2::inside(const Vector2& pt) const
{
	if (pt.x < mn.x || pt.x > mx.x)
		return false;
	if (pt.y < mn.y || pt.y > mx.y)
		return false;

	return true;
}

bool Aabb2::intersectSegment(const Vector2& p1, const Vector2& p2, float& outDistance) const
{
	float enter = 0.0f, exit = 1.0f;

	if (!intersectSegment1D(mn.x, mx.x, p1.x, p2.x, enter, exit))
		return false;
	if (!intersectSegment1D(mn.y, mx.y, p1.y, p2.y, enter, exit))
		return false;

	outDistance = enter;
	return true;
}

}
