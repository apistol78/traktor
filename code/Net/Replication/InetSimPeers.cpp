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

InetSimPeers::InetSimPeers(IReplicatorPeers* peers)
:	m_peers(peers)
,	m_noisyTime(0.0)
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
		SendQueueItem* s = m_sendQueue.front();
		T_ASSERT (s);

		if (s->time + 0.2 > m_noisyTime)
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

	m_noisyTime += m_timer.getDeltaTime() + (m_random.nextDouble() * 0.01 - 0.005);
	return true;
}

void InetSimPeers::setStatus(uint8_t status)
{
	m_peers->setStatus(status);
}

void InetSimPeers::setConnectionState(uint64_t connectionState)
{
	m_peers->setConnectionState(connectionState);
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
	for (;;)
	{
		int32_t result = m_peers->receive(data, size, outFromHandle);
		if (result <= 0)
			return result;

		uint8_t state = m_state[outFromHandle];
		if ((state & 0x02) == 0x00)
			return result;
	}
}

bool InetSimPeers::send(handle_t handle, const void* data, int32_t size, bool reliable)
{
	uint8_t state = m_state[handle];
	if ((state & 0x01) == 0x01)
		return false;

	if (m_random.nextDouble() <= 0.01)
		return true;

	SendQueueItem* s = new SendQueueItem();
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
