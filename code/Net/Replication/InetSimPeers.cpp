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

InetSimPeers::InetSimPeers(IReplicatorPeers* peers, int32_t latency)
:	m_peers(peers)
,	m_noisyTime(0.0)
,	m_latency(double(latency) / 1000.0)
{
	m_timer.start();
}

InetSimPeers::~InetSimPeers()
{
	destroy();
}

void InetSimPeers::destroy()
{
	safeDestroy(m_peers);
}

bool InetSimPeers::update()
{
	if (!m_peers->update())
		return false;

	while (!m_sendQueue.empty())
	{
		QueueItem* s = m_sendQueue.front();
		T_ASSERT (s);

		if (s->time + m_latency > m_noisyTime)
			break;

		m_peers->send(
			s->handle,
			s->data,
			s->size,
			s->reliable
		);

		m_sendQueue.pop_front();
		delete s;
	}

	for (;;)
	{
		handle_t handle;
		uint8_t data[2048];
		int32_t size = sizeof(data);

		int32_t result = m_peers->receive(data, size, handle);
		if (result <= 0)
			break;

		QueueItem* r = new QueueItem();
		r->time = m_timer.getElapsedTime();
		r->handle = handle;
		r->size = result;
		r->reliable = true;

		std::memcpy(r->data, data, result);

		m_receiveQueue.push_back(r);
	}

	m_noisyTime = m_timer.getElapsedTime();
	return true;
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
	if (m_receiveQueue.empty())
		return 0;

	QueueItem* r = m_receiveQueue.front();
	if (r->time + m_latency > m_noisyTime)
		return 0;

	size = std::min(size, r->size);

	std::memcpy(data, r->data, size);
	outFromHandle = r->handle;

	delete r;
	m_receiveQueue.pop_front();

	return size;
}

bool InetSimPeers::send(handle_t handle, const void* data, int32_t size, bool reliable)
{
	uint8_t state = m_state[handle];
	if ((state & 0x01) == 0x01)
		return false;

	if (m_random.nextDouble() <= 0.01)
		return true;

	QueueItem* s = new QueueItem();
	s->time = m_timer.getElapsedTime();
	s->handle = handle;
	s->size = size;
	s->reliable = reliable;

	std::memcpy(s->data, data, size);

	m_sendQueue.push_back(s);
	return true;
}

void InetSimPeers::setPeerConnectionState(handle_t peer, bool sendEnable, bool receiveEnable)
{
	m_state[peer] = ((!sendEnable) ? 0x01 : 0x00) | ((!receiveEnable) ? 0x02 : 0x00);
}

	}
}
