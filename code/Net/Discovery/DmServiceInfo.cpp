#include "Net/Discovery/DmServiceInfo.h"
#include "Net/Discovery/IService.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.net.DmServiceInfo", DmServiceInfo, IDiscoveryMessage)

DmServiceInfo::DmServiceInfo(const Guid& sessionGuid, IService* service)
:	m_sessionGuid(sessionGuid)
,	m_service(service)
{
}

bool DmServiceInfo::serialize(Serializer& s)
{
	s >> Member< Guid >(L"sessionGuid", m_sessionGuid);
	s >> MemberRef< IService >(L"service", m_service);
	return true;
}

	}
}
