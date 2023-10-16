/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Unit.h"

#if defined(max)
#	undef max
#endif

 // import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

/*! Size
 * \ingroup UI
 */
template < typename ValueType >
class T_DLLCLASS SizeBase
{
public:
	ValueType cx = ValueType(0);
	ValueType cy = ValueType(0);

	SizeBase() = default;

	SizeBase(ValueType x, ValueType y)
	:	cx(x)
	,	cy(y)
	{
	}

	SizeBase(const std::pair< ValueType, ValueType >& pr)
	:	cx(pr.first)
	,	cy(pr.second)
	{
	}

	SizeBase(const SizeBase< ValueType >& size)
	:	cx(size.cx)
	,	cy(size.cy)
	{
	}

	SizeBase< ValueType > operator - () const
	{
		return SizeBase< ValueType >(-cx, -cy);
	}

	SizeBase< ValueType > operator + (const SizeBase< ValueType >& r) const
	{
		return SizeBase< ValueType >(cx + r.cx, cy + r.cy);
	}

	SizeBase< ValueType >& operator += (const SizeBase< ValueType >& r)
	{
		cx += r.cx;
		cy += r.cy;
		return *this;
	}

	SizeBase< ValueType > operator - (const SizeBase< ValueType >& r) const
	{
		return SizeBase< ValueType >(cx - r.cx, cy - r.cy);
	}

	SizeBase< ValueType >& operator -= (const SizeBase< ValueType >& r)
	{
		cx -= r.cx;
		cy -= r.cy;
		return *this;
	}

	SizeBase< ValueType > operator * (const SizeBase< ValueType >& r) const
	{
		return SizeBase< ValueType >(cx * r.cx, cy * r.cy);
	}

	SizeBase< ValueType >& operator *= (const SizeBase< ValueType >& r)
	{
		cx *= r.cx;
		cy *= r.cy;
		return *this;
	}

	SizeBase< ValueType > operator / (const SizeBase< ValueType >& r) const
	{
		return SizeBase< ValueType >(cx / r.cx, cy / r.cy);
	}

	SizeBase< ValueType >& operator /= (const SizeBase< ValueType >& r)
	{
		cx /= r.cx;
		cy /= r.cy;
		return *this;
	}

	bool operator == (const SizeBase< ValueType >& r) const
	{
		return cx == r.cx && cy == r.cy;
	}

	bool operator != (const SizeBase< ValueType >& r) const
	{
		return cx != r.cx || cy != r.cy;
	}
};

typedef SizeBase< int32_t > Size;
typedef SizeBase< Unit > UnitSize;

}
