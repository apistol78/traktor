/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Llm/Utf8.h"

namespace traktor::llm
{
namespace
{

const wchar_t c_replacement = 0xfffd;

/*! Bytes in the sequence \a lead introduces, or zero if it introduces none.
 *
 * Rejects the overlong two byte forms and the lead bytes above U+10FFFF,
 * which are not valid UTF-8 however they continue.
 */
uint32_t getSequenceLength(uint8_t lead)
{
	if (lead < 0x80)
		return 1;
	else if (lead >= 0xc2 && lead <= 0xdf)
		return 2;
	else if (lead >= 0xe0 && lead <= 0xef)
		return 3;
	else if (lead >= 0xf0 && lead <= 0xf4)
		return 4;
	else
		return 0;
}

bool isContinuation(uint8_t byte)
{
	return (byte & 0xc0) == 0x80;
}

/*! Decode at \a offset; returns the length consumed, or zero if truncated. */
uint32_t decodeAt(const std::string& text, size_t offset, uint32_t& outCodePoint)
{
	const uint8_t lead = (uint8_t)text[offset];
	const uint32_t length = getSequenceLength(lead);

	if (length == 0)
		return 0;

	if (offset + length > text.size())
		return 0;

	uint32_t codePoint = lead & (0xff >> (length + 1));
	if (length == 1)
		codePoint = lead;

	for (uint32_t i = 1; i < length; ++i)
	{
		const uint8_t byte = (uint8_t)text[offset + i];
		if (!isContinuation(byte))
			return 0;
		codePoint = (codePoint << 6) | (byte & 0x3f);
	}

	// Overlong forms, surrogates and out of range values are all malformed.
	if (length == 3 && codePoint < 0x800)
		return 0;
	if (length == 4 && codePoint < 0x10000)
		return 0;
	if (codePoint >= 0xd800 && codePoint <= 0xdfff)
		return 0;
	if (codePoint > 0x10ffff)
		return 0;

	outCodePoint = codePoint;
	return length;
}

}

size_t getValidUtf8Length(const std::string& text)
{
	size_t i = 0;
	while (i < text.size())
	{
		uint32_t codePoint = 0;
		const uint32_t length = decodeAt(text, i, codePoint);

		if (length > 0)
		{
			i += length;
			continue;
		}

		// A valid lead with room still missing may yet be completed; anything
		// else is broken for good and stays in the prefix.
		const uint32_t expected = getSequenceLength((uint8_t)text[i]);
		if (expected > 0 && i + expected > text.size())
			return i;

		++i;
	}

	return text.size();
}

std::wstring widenUtf8(const std::string& text)
{
	std::wstring result;
	result.reserve(text.size());

	size_t i = 0;
	while (i < text.size())
	{
		uint32_t codePoint = 0;
		const uint32_t length = decodeAt(text, i, codePoint);

		if (length == 0)
		{
			// Always advance, so malformed input cannot stall the loop.
			result += c_replacement;
			++i;
			continue;
		}

		if (sizeof(wchar_t) >= 4 || codePoint <= 0xffff)
			result += (wchar_t)codePoint;
		else
		{
			// Narrow wide characters need a surrogate pair.
			const uint32_t value = codePoint - 0x10000;
			result += (wchar_t)(0xd800 + (value >> 10));
			result += (wchar_t)(0xdc00 + (value & 0x3ff));
		}

		i += length;
	}

	return result;
}

}
