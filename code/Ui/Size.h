/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"

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
class T_DLLCLASS Size
{
public:
	int32_t cx = 0;
	int32_t cy = 0;

	Size() = default;

	Size(int32_t x, int32_t y)
	:	cx(x)
	,	cy(y)
	{
	}

	Size(const std::pair< int32_t, int32_t >& pr)
	:	cx(pr.first)
	,	cy(pr.second)
	{
	}

	Size(const Size& size)
	:	cx(size.cx)
	,	cy(size.cy)
	{
	}

	Size operator - () const
	{
		return Size(-cx, -cy);
	}

	Size operator + (const Size& r) const
	{
		return Size(cx + r.cx, cy + r.cy);
	}

	Size& operator += (const Size& r)
	{
		cx += r.cx;
		cy += r.cy;
		return *this;
	}

	Size operator - (const Size& r) const
	{
		return Size(cx - r.cx, cy - r.cy);
	}

	Size& operator -= (const Size& r)
	{
		cx -= r.cx;
		cy -= r.cy;
		return *this;
	}

	bool operator == (const Size& r) const
	{
		return cx == r.cx && cy == r.cy;
	}

	bool operator != (const Size& r) const
	{
		return cx != r.cx || cy != r.cy;
	}

	static Size max() { return Size(65535, 65535); }
};

}
