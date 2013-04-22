#ifndef traktor_net_IReplicatorPeers_H
#define traktor_net_IReplicatorPeers_H

#include "Core/Object.h"
#include "Net/Replication/ReplicatorTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

class T_DLLCLASS IReplicatorPeers : public Object
{
	T_RTTI_CLASS;

public:
	virtual void destroy() = 0;

	virtual int32_t update() = 0;

	// \{

	virtual std::wstring getName() const = 0;

	virtual uint64_t getGlobalId() const = 0;

	virtual handle_t getPrimaryPeerHandle() const = 0;

	//! \}

	// \{

	virtual uint32_t getPeerHandles(std::vector< handle_t >& outPeerHandles) const = 0;

	virtual std::wstring getPeerName(handle_t handle) const = 0;

	virtual uint64_t getPeerGlobalId(handle_t handle) const = 0;

	virtual int32_t receive(void* data, int32_t size, handle_t& outFromHandle) = 0;

	virtual bool send(handle_t handle, const void* data, int32_t size, bool reliable) = 0;

	//! \}
};

	}
}

#endif	// traktor_net_IReplicatorPeers_H
