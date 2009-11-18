#include "Net/Discovery/DmFindServices.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberType.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.net.DmFindServices", 0, DmFindServices, IDiscoveryMessage)

DmFindServices::DmFindServices(const Guid& sessionGuid, const TypeInfo* serviceType)
:	m_sessionGuid(sessionGuid)
,	m_serviceType(serviceType)
{
}

bool DmFindServices::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"sessionGuid", m_sessionGuid);
	s >> MemberType(L"serviceType", m_serviceType);
	return true;
}

	}
}
