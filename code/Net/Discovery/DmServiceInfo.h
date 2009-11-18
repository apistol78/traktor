#ifndef traktor_net_DmServiceInfo_H
#define traktor_net_DmServiceInfo_H

#include "Net/Discovery/IDiscoveryMessage.h"
#include "Core/Guid.h"

namespace traktor
{
	namespace net
	{

class IService;

class DmServiceInfo : public IDiscoveryMessage
{
	T_RTTI_CLASS;

public:
	DmServiceInfo(const Guid& sessionGuid = Guid(), IService* service = 0);

	const Guid& getSessionGuid() const { return m_sessionGuid; }

	Ref< IService > getService() const { return m_service; }

	virtual bool serialize(ISerializer& s);

private:
	Guid m_sessionGuid;
	Ref< IService > m_service;
};

	}
}

#endif	// traktor_net_DmServiceInfo_H
