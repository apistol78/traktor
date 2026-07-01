/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Io/IStream.h"
#include "Core/Io/StringReader.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.StringReader", StringReader, Object)

StringReader::StringReader(IStream* stream, IEncoding* encoding)
:	m_stream(stream)
,	m_encoding(encoding)
,	m_head(0)
,	m_tail(0)
{
}

wchar_t StringReader::readChar()
{
	wchar_t ch;

	if (m_tail - m_head < (int32_t)sizeof(m_buffer))
	{
		// Buffer's tail reached the end; slide unread bytes to the front so
		// we can refill. This only happens once every ~sizeof(m_buffer) chars
		// instead of on every character decode.
		if (m_tail == (int32_t)sizeof(m_buffer) && m_head > 0)
		{
			std::memmove(&m_buffer[0], &m_buffer[m_head], m_tail - m_head);
			m_tail -= m_head;
			m_head = 0;
		}

		const int64_t result = m_stream->read(&m_buffer[m_tail], sizeof(m_buffer) - m_tail);
		if (result > 0)
			m_tail += (int32_t)result;
		else if (m_tail - m_head <= 0)
			return 0;
	}

	T_ASSERT(m_tail - m_head > 0);

	const int32_t result = m_encoding->translate(&m_buffer[m_head], m_tail - m_head, ch);
	if (result <= 0)
		return 0;

	m_head += result;
	return ch;
}

int64_t StringReader::readAll(std::wstring& out)
{
	out.resize(0);

	std::wstring line;
	bool any = false;
	while (readLine(line) >= 0)
	{
		if (any)
			out.push_back(L'\n');
		out.append(line);
		any = true;
	}

	return any ? (int64_t)out.length() : -1;
}

int64_t StringReader::readLine(std::wstring& out)
{
	wchar_t ch;

	out.reserve(200);
	out.resize(0);

	for (;;)
	{
		if (m_tail - m_head < (int32_t)sizeof(m_buffer))
		{
			// Compact when the buffer tail has reached the end.
			if (m_tail == (int32_t)sizeof(m_buffer) && m_head > 0)
			{
				std::memmove(&m_buffer[0], &m_buffer[m_head], m_tail - m_head);
				m_tail -= m_head;
				m_head = 0;
			}

			int64_t result = -1;
			if (m_stream)
			{
				result = m_stream->read(&m_buffer[m_tail], sizeof(m_buffer) - m_tail);
				if (result < 0)
					m_stream = nullptr;
			}
			if (result > 0)
				m_tail += (int32_t)result;
			else if (m_tail - m_head <= 0 && out.empty())
			{
				out.clear();
				return -1;
			}
		}

		if (m_tail - m_head <= 0)
			break;

		const int32_t result = m_encoding->translate(&m_buffer[m_head], m_tail - m_head, ch);
		if (result <= 0)
		{
			// Need more bytes in buffer; loop to read another byte if room remains.
			if (m_stream != nullptr && (m_tail - m_head) < (int32_t)sizeof(m_buffer))
				continue;
			else
				return -1;
		}

		m_head += result;

		if (ch == L'\n')
			break;
		else if (ch != L'\r')
			out.push_back(ch);
	}

	return (int64_t)out.length();
}

}
