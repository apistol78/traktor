#ifndef traktor_net_InetSimPeers_H
#define traktor_net_InetSimPeers_H

#include <list>
#include "Core/Math/Random.h"
#include "Core/Thread/Event.h"
#include "Core/Thread/Semaphore.h"
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

class Thread;

	namespace net
	{

class T_DLLCLASS InetSimPeers : public IReplicatorPeers
{
	T_RTTI_CLASS;

public:
	InetSimPeers(
		IReplicatorPeers* peers,
		float latencyMin,
		float latencyMax,
		float packetLossRate
	);

	virtual ~InetSimPeers();

	virtual void destroy();

	virtual int32_t update();

	virtual std::wstring getName() const;

	virtual uint64_t getGlobalId() const;

	virtual handle_t getPrimaryPeerHandle() const;

	virtual uint32_t getPeerHandles(std::vector< handle_t >& outPeerHandles) const;

	virtual std::wstring getPeerName(handle_t handle) const;

	virtual uint64_t getPeerGlobalId(handle_t handle) const;

	virtual int32_t receive(void* data, int32_t size, handle_t& outFromHandle);

	virtual bool send(handle_t handle, const void* data, int32_t size, bool reliable);

private:
	struct Packet
	{
		float T;
		handle_t handle;
		uint8_t* data;
		int32_t size;
		bool reliable;
	};

	Ref< IReplicatorPeers > m_peers;
	float m_latencyMin;
	float m_latencyMax;
	float m_packetLossRate;
	Timer m_timer;
	Random m_random;
	std::list< Packet > m_tx;
	Thread* m_txThread;
	Event m_txEvent;
	Semaphore m_txLock;

	void threadTx();
};

	}
}

#endif	// traktor_net_InetSimPeers_H
