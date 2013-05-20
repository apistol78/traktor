#include <cstring>
#include "Core/Math/Float.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Net/Replication/InetSimPeers.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.InetSimPeers", InetSimPeers, IReplicatorPeers)

InetSimPeers::InetSimPeers(
	IReplicatorPeers* peers,
	float latencyMin,
	float latencyMax,
	float packetLossRate
)
:	m_peers(peers)
,	m_latencyMin(latencyMin)
,	m_latencyMax(latencyMax)
,	m_packetLossRate(packetLossRate)
,	m_txThread(0)
{
	m_txThread = ThreadManager::getInstance().create(makeFunctor(this, &InetSimPeers::threadTx));
	m_txThread->start();
}

InetSimPeers::~InetSimPeers()
{
	destroy();
}

void InetSimPeers::destroy()
{
	if (m_txThread)
	{
		m_txThread->stop();
		m_txThread = 0;
	}

	safeDestroy(m_peers);
}

int32_t InetSimPeers::update()
{
	return m_peers->update();
}

void InetSimPeers::setStatus(uint8_t status)
{
	m_peers->setStatus(status);
}

handle_t InetSimPeers::getHandle() const
{
	return m_peers->getHandle();
}

std::wstring InetSimPeers::getName() const
{
	return m_peers->getName();
}

handle_t InetSimPeers::getPrimaryPeerHandle() const
{
	return m_peers->getPrimaryPeerHandle();
}

bool InetSimPeers::setPrimaryPeerHandle(handle_t handle)
{
	return m_peers->setPrimaryPeerHandle(handle);
}

uint32_t InetSimPeers::getPeers(std::vector< PeerInfo >& outPeers) const
{
	return m_peers->getPeers(outPeers);
}

int32_t InetSimPeers::receive(void* data, int32_t size, handle_t& outFromHandle)
{
	return m_peers->receive(data, size, outFromHandle);
}

bool InetSimPeers::send(handle_t handle, const void* data, int32_t size, bool reliable)
{
	if (!reliable && m_packetLossRate >= m_random.nextFloat())
		return true;

	float T = float(m_timer.getElapsedTime()) + lerp(m_latencyMin, m_latencyMax, m_random.nextFloat());

	Packet p;
	p.T = T;
	p.handle = handle;
	p.data = new uint8_t [size];
	p.size = size;
	p.reliable = reliable;

	std::memcpy(p.data, data, size);

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_txLock);
		m_tx.push_back(p);
	}

	m_txEvent.broadcast();

	return true;
}

void InetSimPeers::threadTx()
{
	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
	while (!currentThread->stopped())
	{
		if (!m_txEvent.wait(100))
			continue;

		for (;;)
		{
			Packet p;

			{
				T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_txLock);
				if (m_tx.empty())
					break;

				p = m_tx.front();
				m_tx.pop_front();
			}

			while (p.T > m_timer.getElapsedTime())
				currentThread->sleep(10);

			m_peers->send(p.handle, p.data, p.size, p.reliable);

			delete[] p.data;
		}
	}
}

	}
}
