#ifndef traktor_parade_IReplicatorPeers_H
#define traktor_parade_IReplicatorPeers_H

#include "Core/Object.h"
#include "Parade/Network/ReplicatorTypes.h"

namespace traktor
{
	namespace parade
	{

class IReplicatorPeers : public Object
{
	T_RTTI_CLASS;

public:
	virtual void destroy() = 0;

	virtual void update() = 0;

	virtual uint32_t getPeerHandles(std::vector< handle_t >& outPeerHandles) const = 0;

	virtual std::wstring getPeerName(handle_t handle) const = 0;

	virtual bool receiveAnyPending() = 0;

	virtual bool receive(void* data, uint32_t size, handle_t& outFromHandle) = 0;

	virtual bool sendReady(handle_t handle) = 0;

	virtual bool send(handle_t handle, const void* data, uint32_t size, bool reliable) = 0;

	virtual bool isPrimary() const = 0;
};

	}
}

#endif	// traktor_parade_IReplicatorPeers_H
