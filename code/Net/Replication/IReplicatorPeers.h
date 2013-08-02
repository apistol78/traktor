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
	struct PeerInfo
	{
		handle_t handle;
		std::wstring name;
		uint8_t status;
		bool direct;
		uint64_t connectionState;
		Ref< Object > endSite;

		PeerInfo()
		:	handle(0x00)
		,	status(0)
		,	direct(false)
		,	connectionState(0)
		{
		}
	};

	virtual void destroy() = 0;

	virtual bool update() = 0;

	virtual void setStatus(uint8_t status) = 0;

	virtual void setConnectionState(uint64_t connectionState) = 0;

	//! \{

	virtual handle_t getHandle() const = 0;

	virtual std::wstring getName() const = 0;

	//! \}

	//! \{

	virtual handle_t getPrimaryPeerHandle() const = 0;

	virtual bool setPrimaryPeerHandle(handle_t handle) = 0;

	//! \}

	//! \{

	virtual uint32_t getPeers(std::vector< PeerInfo >& outPeers) const = 0;

	virtual int32_t receive(void* data, int32_t size, handle_t& outFromHandle) = 0;

	virtual bool send(handle_t handle, const void* data, int32_t size, bool reliable) = 0;

	//! \}
};

	}
}

#endif	// traktor_net_IReplicatorPeers_H
