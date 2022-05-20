#pragma once

#include "Ui/Size.h"

namespace traktor
{
	namespace ui
	{

/*! 2D point.
 * \ingroup UI
 */
class Point
{
public:
	int32_t x;
	int32_t y;

	inline Point();

	inline Point(const Point& pnt);

	inline Point(int32_t x_, int32_t y_);

	inline Point operator + (const Size& s) const;

	inline Point operator - (const Size& s) const;

	inline const Point& operator += (const Size& s);

	inline const Point& operator -= (const Size& s);

	inline Size operator - (const Point& p) const;

	inline bool operator == (const Point& p) const;

	inline bool operator != (const Point& p) const;
};

	}
}

#include "Ui/Point.inl"

