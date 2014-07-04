#ifndef traktor_flash_Vector2i_H
#define traktor_flash_Vector2i_H

#include <cmath>
#include "Core/Config.h"
#include "Core/Math/Vector2.h"

namespace traktor
{
	namespace flash
	{

class Vector2i
{
public:
	union
	{
		int32_t e[2];
		struct { int32_t x, y; };
	};

	Vector2i()
	{
	}

	Vector2i(int32_t x_, int32_t y_)
	:	x(x_)
	,	y(y_)
	{
	}

	Vector2 toVector2() const
	{
		return Vector2(float(x), float(y));
	}

	static Vector2i fromVector2(const Vector2& v)
	{
		return Vector2i(int32_t(std::floor(v.x)), int32_t(std::floor(v.y)));
	}

	bool operator == (const Vector2i& r) const
	{
		return x == r.x && y == r.y;
	}

	bool operator != (const Vector2i& r) const
	{
		return x != r.x || y != r.y;
	}

	/*extern*/ friend inline Vector2i operator + (const Vector2i& l, const Vector2i& r);

	/*extern*/ friend inline Vector2i operator - (const Vector2i& l, const Vector2i& r);
};

inline Vector2i operator + (const Vector2i& l, const Vector2i& r)
{
	return Vector2i(l.x + r.x, l.x + r.x);
}

inline Vector2i operator - (const Vector2i& l, const Vector2i& r)
{
	return Vector2i(l.x - r.x, l.x - r.x);
}

	}
}

#endif	// traktor_flash_Vector2i_H
