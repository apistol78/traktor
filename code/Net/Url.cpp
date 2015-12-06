#include "Net/Url.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/Path.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.Url", Url, Object)

Url::Url()
:	m_valid(false)
{
}
		
Url::Url(const std::wstring& spec)
{
	set(spec);
}

bool Url::set(const std::wstring& spec)
{
	size_t i, j;
	
	m_valid = false;
	m_host = L"";
	m_path = L"";
	m_file = L"";
	m_query = L"";
	m_ref = L"";
	m_userInfo = L"";
	
	if ((i = spec.find(L':')) == std::wstring::npos)
		return false;
	
	m_protocol = spec.substr(0, i);
	m_port = getDefaultPort();

	std::wstring specific = spec.substr(i + 1);
	if (specific.substr(0, 2) == L"//")
	{
		specific = specific.substr(2);

		// Extract query.
		if ((i = specific.rfind(L'?')) != std::wstring::npos)
		{
			m_query = specific.substr(i + 1);
			specific = specific.substr(0, i);
		}

		// Extract reference.
		if ((i = specific.rfind(L'#')) != std::wstring::npos)
		{
			m_ref = specific.substr(i + 1);
			specific = specific.substr(0, i);
		}

		// Extract authority.
		if ((i = specific.find(L'/')) != std::wstring::npos)
		{
			std::wstring authority = specific.substr(0, i);
			m_path = specific.substr(i);
			
			// Get filename from path.
			size_t p = m_path.find_last_of('/');
			m_file = (p != std::wstring::npos) ? m_path.substr(p + 1) : m_path;

			// Extract user information.
			if ((j = authority.find(L'@')) != std::wstring::npos)
			{
				m_userInfo = authority.substr(0, j);
				authority = authority.substr(j + 1);
			}

			// Extract host and port.
			if ((j = authority.find(L':')) != std::wstring::npos)
			{
				m_host = authority.substr(0, j);
				m_port = parseString< uint16_t >(authority.substr(j + 1));
			}
			else
			{
				m_host = authority;
			}
		}
		else
		{
			m_path = L"/";

			// Extract host and port.
			if ((j = specific.find(L':')) != std::wstring::npos)
			{
				m_host = specific.substr(0, j);
				m_port = parseString< uint16_t >(specific.substr(j + 1));
			}
			else
			{
				m_host = specific;
			}
		}
	}
	else
	{
		// Non-generic form; save as path.
		m_path = specific;
		m_file = Path(m_path).getFileName();
	}

	m_valid = true;
	return true;
}

bool Url::valid() const
{
	return m_valid;
}

int32_t Url::getDefaultPort() const
{
	if (m_protocol == L"http")
		return 80;
	else if (m_protocol == L"ftp")
		return 21;
	else
		return 0;
}

std::wstring Url::getFile() const
{
	return m_file;
}

std::wstring Url::getHost() const
{
	return m_host;
}

std::wstring Url::getPath() const
{
	return m_path;
}

int32_t Url::getPort() const
{
	return m_port;
}

std::wstring Url::getProtocol() const
{
	return m_protocol;
}

std::wstring Url::getQuery() const
{
	return m_query;
}

std::wstring Url::getRef() const
{
	return m_ref;
}

std::wstring Url::getUserInfo() const
{
	return m_userInfo;
}

std::wstring Url::getString() const
{
	StringOutputStream ss;
	ss << getProtocol() << L"://" << getHost();
	
	if (getPort() != getDefaultPort())
		ss << L":" << m_port;

	std::wstring path = getPath();
	if (!path.empty())
		ss << path;

	std::wstring query = getQuery();
	if (!query.empty())
		ss << L"?" + query;

	return ss.str();
}

std::wstring Url::encode(const std::vector< uint8_t >& bytes)
{
	return bytes.size() > 0 ? encode(&bytes[0], uint32_t(bytes.size())) : L"";
}

std::wstring Url::encode(const uint8_t* bytes, uint32_t len)
{
	StringOutputStream ss;
	for (uint32_t i = 0; i < len; ++i)
	{
		uint8_t ch = bytes[i];
		if (
			(ch >= '0' && ch <= '9') ||
			(ch >= 'a' && ch <= 'z') ||
			(ch >= 'A' && ch <= 'Z') ||
			ch == '.' || ch == ',' || ch == '-' || ch == '_' || ch == '~'
		)
			ss.put(wchar_t(ch));
		else
		{
			const wchar_t c_hex[] = { L"0123456789ABCDEF" };
			ss.put('%');
			ss.put(c_hex[(ch) >> 4]);
			ss.put(c_hex[(ch) & 15]);
		}
	}
	return ss.str();
}

std::wstring Url::encode(const std::wstring& text)
{
	std::string s = wstombs(Utf8Encoding(), text);
	return encode((const uint8_t*)s.c_str(), s.length());
}

std::vector< uint8_t > Url::decodeBytes(const std::wstring& text)
{
	std::vector< uint8_t > bytes;
	for (size_t i = 0; i < text.length(); )
	{
		if (text[i] == L'%' && i < text.length() - 2)
		{
			char hex[] = { char(text[i + 1]), char(text[i + 2]), 0 };
			bytes.push_back(uint8_t(std::strtol(hex, 0, 16)));
			i += 3;
		}
		else
		{
			bytes.push_back(char(text[i]));
			i++;
		}
	}
	return bytes;
}

std::wstring Url::decodeString(const std::wstring& text)
{
	std::vector< wchar_t > chrs;
	for (size_t i = 0; i < text.length(); )
	{
		if (text[i] == L'%' && i < text.length() - 2)
		{
			char hex[] = { char(text[i + 1]), char(text[i + 2]), 0 };
			chrs.push_back(wchar_t(std::strtol(hex, 0, 16)));
			i += 3;
		}
		else
		{
			chrs.push_back(wchar_t(text[i]));
			i++;
		}
	}
	return std::wstring(chrs.begin(), chrs.end());
}

	}
}
