#include "Net/Discovery/DmServiceInfo.h"
#include "Net/Discovery/IService.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.net.DmServiceInfo", 1, DmServiceInfo, IDiscoveryMessage)

DmServiceInfo::DmServiceInfo(const Guid& managerGuid, const Guid& serviceGuid, IService* service)
:	m_managerGuid(managerGuid)
,	m_serviceGuid(serviceGuid)
,	m_service(service)
{
}

bool DmServiceInfo::serialize(ISerializer& s)
{
	if (s.getVersion() < 1)
		return false;

	s >> Member< Guid >(L"managerGuid", m_managerGuid);
	s >> Member< Guid >(L"serviceGuid", m_serviceGuid);
	s >> MemberRef< IService >(L"service", m_service);

	return true;
}

	}
}
