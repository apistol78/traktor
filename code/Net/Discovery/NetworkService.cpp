#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Net/Discovery/NetworkService.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.net.NetworkService", 0, NetworkService, IService)

NetworkService::NetworkService()
{
}

NetworkService::NetworkService(
	const std::wstring& type,
	const std::wstring& host,
	const std::wstring& description
)
:	m_type(type)
,	m_host(host)
,	m_description(description)
{
}

const std::wstring& NetworkService::getType() const
{
	return m_type;
}

const std::wstring& NetworkService::getHost() const
{
	return m_host;
}

std::wstring NetworkService::getDescription() const
{
	return m_description;
}

bool NetworkService::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"type", m_type);
	s >> Member< std::wstring >(L"host", m_host);
	s >> Member< std::wstring >(L"description", m_description);
	return true;
}

	}
}
