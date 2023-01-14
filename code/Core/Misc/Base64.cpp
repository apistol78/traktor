/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/Base64.h"

namespace traktor
{
	namespace
	{

char enc(uint8_t uc)
{
	if (uc < 26)
		return 'A' + uc;
	if (uc < 52)
		return 'a' + (uc - 26);
	if (uc < 62)
		return '0' + (uc - 52);
	return (uc == 62) ? '+' : '/';
}

uint8_t dec(wchar_t c)
{
	if (c >= L'A' && c <= L'Z')
		return c - L'A';
	if (c >= L'a' && c <= L'z')
		return c - L'a' + 26;
	if (c >= L'0' && c <= L'9')
		return c - L'0' + 52;
	return (c == L'+') ? 62 : 63;
}

bool isBase64(wchar_t c)
{
	if (c >= L'A' && c <= L'Z')
		return true;
	if (c >= L'a' && c <= L'z')
		return true;
	if (c >= L'0' && c <= L'9')
		return true;
	if (c == L'+' || c == L'/' || c == L'=')
		return true;
	return false;
}

bool nextBase64(const std::wstring& b64, size_t& pos, wchar_t& outChar)
{
	const size_t ln = b64.size();
	while (pos < ln)
	{
		if (isBase64(b64[pos]))
		{
			outChar = b64[pos];
			pos++;
			return true;
		}
		pos++;
	}
	return false;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Base64", Base64, Object)

std::wstring Base64::encode(const uint8_t* data, uint32_t size, bool insertCrLf) const
{
	StringOutputStream ss;
	for (size_t i = 0; i < size; i += 3)
	{
		uint8_t c1 = 0, c2 = 0, c3 = 0;
		c1 = data[i];
		if (i + 1 < size)
			c2 = data[i + 1];
		if (i + 2 < size)
			c3 = data[i + 2];

		uint8_t uc1 = 0, uc2 = 0, uc3 = 0, uc4 = 0;
		uc1 = c1 >> 2;
		uc2 = ((c1 & 0x3) << 4) | (c2 >> 4);
		uc3 = ((c2 & 0xf) << 2) | (c3 >> 6);
		uc4 = c3 & 0x3f;

		ss << enc(uc1);
		ss << enc(uc2);

		if (i + 1 < size)
			ss << enc(uc3);
		else
			ss << L'=';

		if (i + 2 < size)
			ss << enc(uc4);
		else
			ss << L'=';

		if (insertCrLf && i % (76 / 4 * 3) == 0)
			ss << Endl;
	}
	return ss.str();
}

std::wstring Base64::encode(const AlignedVector< uint8_t >& data, bool insertCrLf) const
{
	return data.size() > 0 ? encode(&data[0], uint32_t(data.size()), insertCrLf) : L"";
}

AlignedVector< uint8_t > Base64::decode(const std::wstring& b64) const
{
	AlignedVector< uint8_t > data;
	data.reserve(b64.length() / 4);

	for (size_t i = 0; i < b64.length(); )
	{
		wchar_t c1 = L'A', c2 = L'A', c3 = L'A', c4 = L'A';

		nextBase64(b64, i, c1);
		nextBase64(b64, i, c2);
		nextBase64(b64, i, c3);
		nextBase64(b64, i, c4);

		uint8_t uc1 = 0, uc2 = 0, uc3 = 0, uc4 = 0;
		uc1 = dec(c1);
		uc2 = dec(c2);
		uc3 = dec(c3);
		uc4 = dec(c4);

		data.push_back((uc1 << 2) | (uc2 >> 4));
		if (c3 != L'=')
			data.push_back(((uc2 & 0xf) << 4) | (uc3 >> 2));
		if (c4 != L'=')
			data.push_back(((uc3 & 0x3) << 6) | uc4);
	}

	return data;
}

}
