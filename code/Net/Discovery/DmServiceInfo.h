#ifndef traktor_net_DmServiceInfo_H
#define traktor_net_DmServiceInfo_H

#include "Core/Guid.h"
#include "Net/Discovery/IDiscoveryMessage.h"

namespace traktor
{
	namespace net
	{

class IService;

class DmServiceInfo : public IDiscoveryMessage
{
	T_RTTI_CLASS;

public:
	DmServiceInfo(
		const Guid& serviceGuid = Guid(),
		IService* service = 0
	);

	const Guid& getServiceGuid() const { return m_serviceGuid; }

	Ref< IService > getService() const { return m_service; }

	virtual void serialize(ISerializer& s);

private:
	Guid m_serviceGuid;
	Ref< IService > m_service;
};

	}
}

#endif	// traktor_net_DmServiceInfo_H
