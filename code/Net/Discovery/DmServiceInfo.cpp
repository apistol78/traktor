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

DmServiceInfo::DmServiceInfo(const Guid& sessionGuid, IService* service)
:	m_sessionGuid(sessionGuid)
,	m_service(service)
{
}

bool DmServiceInfo::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"sessionGuid", m_sessionGuid);
	s >> MemberRef< IService >(L"service", m_service);
	return true;
}

	}
}
