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

	T_FORCE_INLINE Size();

	T_FORCE_INLINE Size(int32_t x, int32_t y);

	T_FORCE_INLINE Size(const std::pair< int32_t, int32_t >& pr);

	T_FORCE_INLINE Size(const Size& size);

	T_FORCE_INLINE Size operator - () const;

	T_FORCE_INLINE Size operator + (const Size& r) const;

	T_FORCE_INLINE Size& operator += (const Size& r);

	T_FORCE_INLINE Size operator - (const Size& r) const;

	T_FORCE_INLINE Size& operator -= (const Size& r);

	T_FORCE_INLINE bool operator == (const Size& r) const;

	T_FORCE_INLINE bool operator != (const Size& r) const;

	T_FORCE_INLINE static Size max() { return Size(65535, 65535); }
};

}

#include "Ui/Size.inl"
