/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/Utf32Encoding.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Utf32Encoding", Utf32Encoding, IEncoding)

int32_t Utf32Encoding::translate(const wchar_t* chars, int32_t count, uint8_t* out) const
{
	int32_t j = 0;
	for (int32_t i = 0; i < count; ++i)
	{
		wchar_t ch = chars[i];
		out[j++] = uint8_t(ch);
		out[j++] = uint8_t(ch >> 8);
#if !defined(_WIN32)
		out[j++] = uint8_t(ch >> 16);
		out[j++] = uint8_t(ch >> 24);
#else
		out[j++] = 0;
		out[j++] = 0;
#endif
	}
	return j;
}

int32_t Utf32Encoding::translate(const uint8_t in[MaxEncodingSize], int32_t count, wchar_t& out) const
{
#if !defined(_WIN32)
	out = wchar_t(in[3] << 24) | wchar_t(in[2] << 16) | wchar_t(in[1] << 8) | in[0];
#else
	out = wchar_t(in[1] << 8) | in[0];
#endif
	return 4;
}

}
