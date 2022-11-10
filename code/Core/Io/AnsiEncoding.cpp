/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/AnsiEncoding.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.AnsiEncoding", AnsiEncoding, IEncoding)

int32_t AnsiEncoding::translate(const wchar_t* chars, int32_t count, uint8_t* out) const
{
	int32_t j = 0;
	for (int32_t i = 0; i < count; ++i)
	{
		if (chars[i] <= 0xff)
			out[j++] = uint8_t(chars[i] & 0xff);
	}
	return j;
}

int32_t AnsiEncoding::translate(const uint8_t in[MaxEncodingSize], int32_t count, wchar_t& out) const
{
	T_ASSERT(count > 0);
	out = wchar_t(in[0]);
	return 1;
}

}
