/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Size.h"

 // import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

/*! 2D point.
 * \ingroup UI
 */
class T_DLLCLASS Point
{
public:
	int32_t x = 0;
	int32_t y = 0;

	Point() = default;

	Point(const Point& pnt)
	:	x(pnt.x)
	,	y(pnt.y)
	{
	}

	Point(const std::pair< int32_t, int32_t >& pr)
	:	x(pr.first)
	,	y(pr.second)
	{
	}

	Point(int32_t x_, int32_t y_)
	:	x(x_)
	,	y(y_)
	{
	}

	Point operator + (const Size& s) const
	{
		return Point(x + s.cx, y + s.cy);
	}

	Point operator - (const Size& s) const
	{
		return Point(x - s.cx, y - s.cy);
	}

	const Point& operator += (const Size& s)
	{
		x += s.cx; y += s.cy;
		return *this;
	}

	const Point& operator -= (const Size& s)
	{
		x -= s.cx; y -= s.cy;
		return *this;
	}

	Size operator - (const Point& p) const
	{
		return Size(x - p.x, y - p.y);
	}

	bool operator == (const Point& p) const
	{
		return bool(x == p.x && y == p.y);
	}

	bool operator != (const Point& p) const
	{
		return bool(x != p.x || y != p.y);
	}
};

}
