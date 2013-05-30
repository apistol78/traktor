#ifndef traktor_net_RelayPeers_H
#define traktor_net_RelayPeers_H

#include <map>
#include "Net/Replication/IReplicatorPeers.h"

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

class T_DLLCLASS RelayPeers : public IReplicatorPeers
{
	T_RTTI_CLASS;

public:
	RelayPeers(IReplicatorPeers* peers);

	virtual ~RelayPeers();

	virtual void destroy();

	virtual bool update();

	virtual void setStatus(uint8_t status);

	virtual void setConnectionState(uint64_t connectionState);
	
	virtual handle_t getHandle() const;

	virtual std::wstring getName() const;

	virtual handle_t getPrimaryPeerHandle() const;

	virtual bool setPrimaryPeerHandle(handle_t handle);

	virtual uint32_t getPeers(std::vector< PeerInfo >& outPeers) const;

	virtual int32_t receive(void* data, int32_t size, handle_t& outFromHandle);

	virtual bool send(handle_t handle, const void* data, int32_t size, bool reliable);

private:
#pragma pack(1)
	struct Envelope
	{
		uint8_t flags;
		handle_t from;
		handle_t to;
		uint8_t payload[1200];
	};
#pragma pack()

	struct State
	{
		handle_t relayer;
		uint64_t connectionState;

		State()
		:	relayer(0)
		,	connectionState(0)
		{
		}
	};

	Ref< IReplicatorPeers > m_peers;
	std::vector< PeerInfo > m_info;
	mutable std::map< handle_t, State > m_state;
	mutable uint64_t m_connectionState;
	mutable uint64_t m_connectionStateLast;

	bool sendDirect(const Envelope& e, uint32_t payloadSize, bool reliable);

	bool sendRelay(const Envelope& e, uint32_t payloadSize, bool reliable);
};

	}
}

#endif	// traktor_net_RelayPeers_H
