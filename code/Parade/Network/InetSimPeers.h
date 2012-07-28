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

	virtual void update();

	virtual uint32_t getPeerHandles(std::vector< handle_t >& outPeerHandles) const;

	virtual std::wstring getPeerName(handle_t handle) const;

	virtual bool receiveAnyPending();

	virtual bool receive(void* data, uint32_t size, handle_t& outFromHandle);

	virtual bool sendReady(handle_t handle);

	virtual bool send(handle_t handle, const void* data, uint32_t size, bool reliable);

private:
	struct Packet
	{
		float T;
		handle_t handle;
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
