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
template < typename ValueType >
class T_DLLCLASS PointBase
{
public:
	ValueType x = ValueType(0);
	ValueType y = ValueType(0);

	PointBase() = default;

	PointBase(const PointBase< ValueType >& pnt)
	:	x(pnt.x)
	,	y(pnt.y)
	{
	}

	PointBase(const std::pair< ValueType, ValueType >& pr)
	:	x(pr.first)
	,	y(pr.second)
	{
	}

	PointBase(ValueType x_, ValueType y_)
	:	x(x_)
	,	y(y_)
	{
	}

	PointBase< ValueType > operator + (const SizeBase< ValueType >& s) const
	{
		return PointBase< ValueType >(x + s.cx, y + s.cy);
	}

	PointBase< ValueType > operator - (const SizeBase< ValueType >& s) const
	{
		return PointBase< ValueType >(x - s.cx, y - s.cy);
	}

	PointBase< ValueType > operator * (const SizeBase< ValueType >& s) const
	{
		return PointBase< ValueType >(x * s.cx, y * s.cy);
	}

	PointBase< ValueType > operator / (const SizeBase< ValueType >& s) const
	{
		return PointBase< ValueType >(x / s.cx, y / s.cy);
	}

	const PointBase< ValueType >& operator += (const SizeBase< ValueType >& s)
	{
		x += s.cx; y += s.cy;
		return *this;
	}

	const PointBase< ValueType >& operator -= (const SizeBase< ValueType >& s)
	{
		x -= s.cx; y -= s.cy;
		return *this;
	}

	SizeBase< ValueType > operator - (const PointBase< ValueType >& p) const
	{
		return SizeBase< ValueType >(x - p.x, y - p.y);
	}

	bool operator == (const PointBase< ValueType >& p) const
	{
		return bool(x == p.x && y == p.y);
	}

	bool operator != (const PointBase< ValueType >& p) const
	{
		return bool(x != p.x || y != p.y);
	}
};

typedef PointBase< int32_t > Point;
typedef PointBase< Unit > UnitPoint;

}
