#ifndef traktor_net_DmFindServices_H
#define traktor_net_DmFindServices_H

#include "Core/Guid.h"
#include "Net/Discovery/IDiscoveryMessage.h"

namespace traktor
{
	namespace net
	{

class DmFindServices : public IDiscoveryMessage
{
	T_RTTI_CLASS;

public:
	DmFindServices(const Guid& managerGuid = Guid());

	const Guid& getManagerGuid() const { return m_managerGuid; }

	virtual void serialize(ISerializer& s);

private:
	Guid m_managerGuid;
};

	}
}

#endif	// traktor_net_DmFindServices_H
