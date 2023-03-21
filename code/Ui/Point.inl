/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Point.h"

namespace traktor::ui
{

inline Point::Point()
:	x(0)
,	y(0)
{
}

inline Point::Point(const Point& pnt)
:	x(pnt.x)
,	y(pnt.y)
{
}

inline Point::Point(const std::pair< int32_t, int32_t >& pr)
:	x(pr.first)
,	y(pr.second)
{
}

inline Point::Point(int32_t x_, int32_t y_)
:	x(x_)
,	y(y_)
{
}

inline Point Point::dpi96() const
{
	return Point(ui::dpi96(x), ui::dpi96(y));
}

inline Point Point::invdpi96() const
{
	return Point(ui::invdpi96(x), ui::invdpi96(y));
}

inline Point Point::operator + (const Size& s) const
{
	return Point(x + s.cx, y + s.cy);
}

inline Point Point::operator - (const Size& s) const
{
	return Point(x - s.cx, y - s.cy);
}

inline const Point& Point::operator += (const Size& s)
{
	x += s.cx; y += s.cy;
	return *this;
}

inline const Point& Point::operator -= (const Size& s)
{
	x -= s.cx; y -= s.cy;
	return *this;
}

inline Size Point::operator - (const Point& p) const
{
	return Size(x - p.x, y - p.y);
}

inline bool Point::operator == (const Point& p) const
{
	return bool(x == p.x && y == p.y);
}

inline bool Point::operator != (const Point& p) const
{
	return bool(x != p.x || y != p.y);
}

}
