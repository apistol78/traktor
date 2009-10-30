#ifndef traktor_net_IDiscoveryMessage_H
#define traktor_net_IDiscoveryMessage_H

#include "Core/Serialization/Serializable.h"

namespace traktor
{
	namespace net
	{

class IDiscoveryMessage : public Serializable
{
	T_RTTI_CLASS(IDiscoveryMessage)
};

	}
}

#endif	// traktor_net_IDiscoveryMessage_H
