#include "Core/Io/StringOutputStream.h"
#include "Net/Url.h"
#include "Net/Http/HttpRequestParameters.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.HttpRequestParameters", HttpRequestParameters, IHttpRequestContent)

void HttpRequestParameters::set(const std::wstring& key, const std::wstring& value)
{
	if (!value.empty())
		m_parameters[key] = value;
	else
		m_parameters.erase(key);
}

std::wstring HttpRequestParameters::getUrlEncodedContent() const
{
	StringOutputStream ss;
	std::wstring separator = L"";
	for (std::map< std::wstring, std::wstring >::const_iterator i = m_parameters.begin(); i != m_parameters.end(); ++i)
	{
		ss << separator << Url::encode(i->first) << L"=" << Url::encode(i->second);
		separator = L"&";
	}
	return ss.str();
}

	}
}
