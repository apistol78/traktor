/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <string>
#include "Core/Io/IStream.h"
#include "Core/Misc/String.h"
#include "Net/Http/HttpResponse.h"

namespace traktor::net
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.HttpResponse", HttpResponse, Object)

bool HttpResponse::parse(IStream* stream)
{
	if (!stream)
		return false;

	std::wstring response;
	for (int32_t state = 0; state >= 0; )
	{
		char ch;
		if (stream->read(&ch, sizeof(ch)) != sizeof(ch))
			break;

		switch (state)
		{
		case 0:
			if (ch == '\r')
			{
				// "HTTP/1.1 <Code> <Message";

				const size_t sp1 = response.find(' ');
				const size_t sp2 = response.find(' ', sp1 + 1);

				m_statusCode = parseString< int >(response.substr(sp1 + 1, sp2 - sp1));
				m_statusMessage = response.substr(sp2 + 1);

				response = L"";
				state = 1;
			}
			else
				response += ch;
			break;

		case 1:
			if (ch == '\r')
			{
				const size_t p = response.find(L": ");
				if (p != std::wstring::npos)
				{
					std::wstring key = response.substr(0, p);
					std::wstring value = response.substr(p + 2);
					m_values[key] = value;
				}
				response = L"";
				state = 2;
			}
			else
				response += ch;
			break;

		case 2:
			if (ch == '\n')
				state = 3;
			else
			{
				response += ch;
				state = 1;
			}
			break;

		case 3:
			if (ch == '\r')
				state = 4;
			else
			{
				response += ch;
				state = 1;
			}
			break;

		case 4:
			if (ch == '\n')
				state = -1;
			else
			{
				response += ch;
				state = 1;
			}
			break;
		}
	}

	return true;
}

int32_t HttpResponse::getStatusCode() const
{
	return m_statusCode;
}

const std::wstring& HttpResponse::getStatusMessage() const
{
	return m_statusMessage;
}

void HttpResponse::set(const std::wstring& name, const std::wstring& value)
{
	m_values[name] = value;
}

std::wstring HttpResponse::get(const std::wstring& name) const
{
	const auto it = m_values.find(name);
	return it != m_values.end() ? it->second : L"";
}

}
