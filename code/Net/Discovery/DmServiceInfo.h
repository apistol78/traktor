#ifndef traktor_net_DmServiceInfo_H
#define traktor_net_DmServiceInfo_H

#include "Net/Discovery/IDiscoveryMessage.h"

namespace traktor
{
	namespace net
	{

class DmServiceInfo : public IDiscoveryMessage
{
	T_RTTI_CLASS(DmServiceInfo)

public:
	DmServiceInfo(const std::wstring& serviceName = L"");

	virtual bool serialize(Serializer& s);

private:
	std::wstring m_serviceName;
};

	}
}

#endif	// traktor_net_DmServiceInfo_H
