#ifndef traktor_net_DmFindServices_H
#define traktor_net_DmFindServices_H

#include "Net/Discovery/IDiscoveryMessage.h"
#include "Core/Guid.h"

namespace traktor
{
	namespace net
	{

class DmFindServices : public IDiscoveryMessage
{
	T_RTTI_CLASS(DmFindServices)

public:
	DmFindServices(const Guid& sessionGuid = Guid(), const Type* serviceType = 0);

	const Guid& getSessionGuid() const { return m_sessionGuid; }

	const Type* getServiceType() const { return m_serviceType; }

	virtual bool serialize(Serializer& s);

private:
	Guid m_sessionGuid;
	const Type* m_serviceType;
};

	}
}

#endif	// traktor_net_DmFindServices_H
