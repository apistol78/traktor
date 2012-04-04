#ifndef traktor_parade_InetSimPeers_H
#define traktor_parade_InetSimPeers_H

#include <list>
#include "Core/Math/Random.h"
#include "Core/Thread/Event.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Timer/Timer.h"
#include "Parade/Network/IReplicatorPeers.h"

namespace traktor
{

class Thread;

	namespace parade
	{

class InetSimPeers : public IReplicatorPeers
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

	virtual uint32_t getPeerCount() const;

	virtual bool receiveAnyPending();

	virtual bool receive(void* data, uint32_t size, uint32_t& outFromPeer);

	virtual bool sendReady(uint32_t peerId);

	virtual bool send(uint32_t peerId, const void* data, uint32_t size, bool reliable);

private:
	struct Packet
	{
		float T;
		uint32_t peerId;
		uint8_t* data;
		uint32_t size;
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

#endif	// traktor_parade_InetSimPeers_H
