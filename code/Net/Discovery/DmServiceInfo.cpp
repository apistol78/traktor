#include "Net/Discovery/DmServiceInfo.h"
#include "Net/Discovery/IService.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.net.DmServiceInfo", 0, DmServiceInfo, IDiscoveryMessage)

DmServiceInfo::DmServiceInfo(const Guid& serviceGuid, IService* service)
:	m_serviceGuid(serviceGuid)
,	m_service(service)
{
}

void DmServiceInfo::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"serviceGuid", m_serviceGuid);
	s >> MemberRef< IService >(L"service", m_service);
}

	}
}
