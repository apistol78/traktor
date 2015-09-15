#include <string>
#include "Net/Http/HttpResponse.h"
#include "Core/Io/IStream.h"
#include "Core/Misc/String.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.HttpResponse", HttpResponse, Object)

HttpResponse::HttpResponse()
:	m_statusCode(0)
{
}

bool HttpResponse::parse(IStream* stream)
{
	if (!stream)
		return false;

	std::wstring response;
	for (int state = 0; state >= 0; )
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

				size_t sp1 = response.find(' ');
				size_t sp2 = response.find(' ', sp1 + 1);

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
				size_t p = response.find(L": ");
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
	std::map< std::wstring, std::wstring >::const_iterator i = m_values.find(name);
	return i != m_values.end() ? i->second : L"";
}

	}
}
