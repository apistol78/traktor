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
#include "Ui/Display.h"

#if defined(max)
#	undef max
#endif

namespace traktor::ui
{

/*! Size
 * \ingroup UI
 */
class Size
{
public:
	int32_t cx;
	int32_t cy;

	inline Size();

	inline Size(int32_t x, int32_t y);

	inline Size(const std::pair< int32_t, int32_t >& pr);

	inline Size(const Size& size);

	inline Size dpi96() const;

	inline Size invdpi96() const;

	inline Size operator - () const;

	inline Size operator + (const Size& r) const;

	inline Size& operator += (const Size& r);

	inline Size operator - (const Size& r) const;

	inline Size& operator -= (const Size& r);

	inline bool operator == (const Size& r) const;

	inline bool operator != (const Size& r) const;

	inline operator std::pair< int32_t, int32_t >() const;

	inline static Size max() { return Size(65535, 65535); }
};

}

#include "Ui/Size.inl"
