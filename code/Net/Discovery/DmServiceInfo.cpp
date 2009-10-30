#include "Net/Discovery/DmServiceInfo.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.net.DmServiceInfo", DmServiceInfo, IDiscoveryMessage)

DmServiceInfo::DmServiceInfo(const std::wstring& serviceName)
:	m_serviceName(serviceName)
{
}

bool DmServiceInfo::serialize(Serializer& s)
{
	return s >> Member< std::wstring >(L"serviceName", m_serviceName);
}

	}
}
