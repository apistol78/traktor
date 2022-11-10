/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/Utf16Encoding.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Utf16Encoding", Utf16Encoding, IEncoding);

int Utf16Encoding::translate(const wchar_t* chars, int32_t count, uint8_t* out) const
{
	int32_t j = 0;
	for (int32_t i = 0; i < count; ++i)
	{
		wchar_t ch = chars[i];
		out[j++] = uint8_t(ch);
		out[j++] = uint8_t(ch >> 8);
	}
	return j;
}

int Utf16Encoding::translate(const uint8_t in[MaxEncodingSize], int32_t count, wchar_t& out) const
{
	out = wchar_t(in[1] << 8) | in[0];
	return 2;
}

}
