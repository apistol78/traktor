#include "Net/Discovery/DmFindServices.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberType.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.net.DmFindServices", DmFindServices, IDiscoveryMessage)

DmFindServices::DmFindServices(const Guid& sessionGuid, const Type* serviceType)
:	m_sessionGuid(sessionGuid)
,	m_serviceType(serviceType)
{
}

bool DmFindServices::serialize(Serializer& s)
{
	s >> Member< Guid >(L"sessionGuid", m_sessionGuid);
	s >> MemberType(L"serviceType", m_serviceType);
	return true;
}

	}
}
