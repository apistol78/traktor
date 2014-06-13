#ifndef traktor_net_InetSimPeers_H
#define traktor_net_InetSimPeers_H

#include <list>
#include <map>
#include "Core/Math/Random.h"
#include "Core/Timer/Timer.h"
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

class T_DLLCLASS InetSimPeers : public IReplicatorPeers
{
	T_RTTI_CLASS;

public:
	InetSimPeers(IReplicatorPeers* peers, int32_t latency);

	virtual ~InetSimPeers();

	virtual void destroy();

	virtual bool update();

	virtual handle_t getHandle() const;

	virtual std::wstring getName() const;

	virtual handle_t getPrimaryPeerHandle() const;

	virtual bool setPrimaryPeerHandle(handle_t handle);

	virtual uint32_t getPeers(std::vector< PeerInfo >& outPeers) const;

	virtual int32_t receive(void* data, int32_t size, handle_t& outFromHandle);

	virtual bool send(handle_t handle, const void* data, int32_t size, bool reliable);

	void setPeerConnectionState(handle_t peer, bool sendEnable, bool receiveEnable);

private:
	struct QueueItem
	{
		double time;
		handle_t handle;
		uint8_t data[2048];
		int32_t size;
		bool reliable;
	};

	Timer m_timer;
	Random m_random;
	double m_noisyTime;
	double m_latency;
	Ref< IReplicatorPeers > m_peers;
	std::map< handle_t, uint32_t > m_state;
	std::list< QueueItem* > m_sendQueue;
	std::list< QueueItem* > m_receiveQueue;
};

	}
}

#endif	// traktor_net_InetSimPeers_H
