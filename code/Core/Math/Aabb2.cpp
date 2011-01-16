#include <limits>
#include "Core/Math/Aabb2.h"

namespace traktor
{

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

}
