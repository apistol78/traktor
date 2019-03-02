#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Net/Discovery/NetworkService.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.net.NetworkService", 1, NetworkService, IService)

NetworkService::NetworkService()
{
}

NetworkService::NetworkService(
	const std::wstring& type,
	const PropertyGroup* properties
)
:	m_type(type)
,	m_properties(properties)
{
}

const std::wstring& NetworkService::getType() const
{
	return m_type;
}

const PropertyGroup* NetworkService::getProperties() const
{
	return m_properties;
}

void NetworkService::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"type", m_type);
	s >> MemberRef< const PropertyGroup >(L"properties", m_properties);
}

	}
}
