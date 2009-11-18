#ifndef traktor_net_IDiscoveryMessage_H
#define traktor_net_IDiscoveryMessage_H

#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace net
	{

class IDiscoveryMessage : public ISerializable
{
	T_RTTI_CLASS;
};

	}
}

#endif	// traktor_net_IDiscoveryMessage_H
