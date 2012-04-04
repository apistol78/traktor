#ifndef traktor_parade_IReplicatorPeers_H
#define traktor_parade_IReplicatorPeers_H

#include "Core/Object.h"

namespace traktor
{
	namespace parade
	{

class IReplicatorPeers : public Object
{
	T_RTTI_CLASS;

public:
	virtual void destroy() = 0;

	virtual uint32_t getPeerCount() const = 0;

	virtual bool receiveAnyPending() = 0;

	virtual bool receive(void* data, uint32_t size, uint32_t& outFromPeer) = 0;

	virtual bool sendReady(uint32_t peerId) = 0;

	virtual bool send(uint32_t peerId, const void* data, uint32_t size, bool reliable) = 0;
};

	}
}

#endif	// traktor_parade_IReplicatorPeers_H
