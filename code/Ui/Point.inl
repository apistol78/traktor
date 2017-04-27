/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Point.h"

namespace traktor
{
	namespace ui
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

inline Point::Point(int x_, int y_)
:	x(x_)
,	y(y_)
{
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
}
