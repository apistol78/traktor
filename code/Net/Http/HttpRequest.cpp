#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Net/Http/HttpRequest.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.HttpRequest", HttpRequest, Object)

HttpRequest::HttpRequest()
:	m_method(MtUnknown)
{
}

HttpRequest::Method HttpRequest::getMethod() const
{
	return m_method;
}

const std::wstring& HttpRequest::getResource() const
{
	return m_resource;
}

bool HttpRequest::hasValue(const std::wstring& key) const
{
	return bool(m_values.find(key) != m_values.end());
}

void HttpRequest::setValue(const std::wstring& key, const std::wstring& value)
{
	m_values[key] = value;
}

std::wstring HttpRequest::getValue(const std::wstring& key) const
{
	std::map< std::wstring, std::wstring >::const_iterator i = m_values.find(key);
	if (i != m_values.end())
		return i->second;
	else
		return L"";
}

Ref< HttpRequest > HttpRequest::parse(const std::wstring& request)
{
	Ref< HttpRequest > hr = new HttpRequest();
	bool haveMethod = false;

	StringSplit< std::wstring > split(request, L"\n\r");
	for (StringSplit< std::wstring >::const_iterator i = split.begin(); i != split.end(); ++i)
	{
		std::wstring line = trim(*i);
		if (line.empty())
			continue;

		if (!haveMethod)
		{
			std::vector< std::wstring > tmp;
			Split< std::wstring >::any(line, L" \t", tmp);

			if (tmp.size() >= 2)
			{
				if (tmp[0] == L"GET")
					hr->m_method = MtGet;
				else if (tmp[0] == L"HEAD")
					hr->m_method = MtHead;
				else if (tmp[0] == L"POST")
					hr->m_method = MtPost;
				else if (tmp[0] == L"PUT")
					hr->m_method = MtPut;
				else if (tmp[0] == L"DELETE")
					hr->m_method = MtDelete;
				else if (tmp[0] == L"TRACE")
					hr->m_method = MtTrace;
				else if (tmp[0] == L"OPTIONS")
					hr->m_method = MtOptions;
				else if (tmp[0] == L"CONNECT")
					hr->m_method = MtConnect;
				else if (tmp[0] == L"PATCH")
					hr->m_method = MtPatch;
				else
					return 0;

				hr->m_resource = tmp[1];
			}
			else
				return 0;

			haveMethod = true;
		}
		else
		{
			size_t p = line.find(L": ");
			if (p != std::wstring::npos)
				hr->m_values[line.substr(0, p)] = line.substr(p + 2);
		}
	}

	return haveMethod ? hr : 0;
}

	}
}
