/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Core/Io/IEncoding.h"
#include "Core/Misc/TString.h"

namespace traktor
{

std::wstring mbstows(const IEncoding& encoding, const std::string_view& s)
{
	std::wstring es;
	wchar_t ec;
	int32_t r;

	es.reserve(s.length());

	const uint8_t* cs = reinterpret_cast< const uint8_t* >(s.data());
	for (uint32_t i = 0; i < s.length(); )
	{
		const uint32_t nb = std::min< uint32_t >(IEncoding::MaxEncodingSize, uint32_t(s.length() - i));
		if ((r = encoding.translate(&cs[i], nb, ec)) < 0)
			break;

		es.push_back(ec);
		i += r;
	}

	return es;
}

std::string wstombs(const IEncoding& encoding, const std::wstring_view& s)
{
	uint8_t buf[IEncoding::MaxEncodingSize];
	std::string out;

	const size_t ln = s.length();
	out.reserve(ln);

	for (size_t i = 0; i < ln; ++i)
	{
		const int32_t nbuf = encoding.translate(&s[i], 1, buf);
		if (nbuf <= 0)
			continue;

		out += std::string_view((const char*)buf, nbuf);
	}

	return out;
}

}
