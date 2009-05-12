#include "Net/Http/HttpRequest.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.HttpRequest", HttpRequest, Object)

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
	if (i == m_values.end())
		return L"";
	return i->second;
}

HttpRequest* HttpRequest::parse(const std::wstring& request)
{
	return 0;
}

	}
}
