#include <algorithm>
#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Net/Replication/RelayPeers.h"

namespace traktor
{
	namespace net
	{
		namespace
		{

#if 0
#	define T_RELAY_DEBUG(x) traktor::log::info << x << traktor::Endl
#else
#	define T_RELAY_DEBUG(x)
#endif

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.RelayPeers", RelayPeers, IReplicatorPeers)

RelayPeers::RelayPeers(IReplicatorPeers* peers)
:	m_peers(peers)
{
}

RelayPeers::~RelayPeers()
{
	destroy();
}

void RelayPeers::destroy()
{
	safeDestroy(m_peers);
}

int32_t RelayPeers::update()
{
	T_ASSERT (m_peers);

	m_info.resize(0);
	m_peers->getPeers(m_info);

	return m_peers->update();
}

void RelayPeers::setStatus(uint8_t status)
{
	m_peers->setStatus(status);
}

handle_t RelayPeers::getHandle() const
{
	return m_peers->getHandle();
}

std::wstring RelayPeers::getName() const
{
	return m_peers->getName();
}

handle_t RelayPeers::getPrimaryPeerHandle() const
{
	return m_peers->getPrimaryPeerHandle();
}

bool RelayPeers::setPrimaryPeerHandle(handle_t handle)
{
	return m_peers->setPrimaryPeerHandle(handle);
}

uint32_t RelayPeers::getPeers(std::vector< PeerInfo >& outPeers) const
{
	outPeers = m_info;

	for (std::vector< PeerInfo >::iterator i = outPeers.begin(); i != outPeers.end(); ++i)
		i->relayed = bool((m_state[i->handle].flags & SfRelayed) != 0);

	return outPeers.size();
}

int32_t RelayPeers::receive(void* data, int32_t size, handle_t& outFromHandle)
{
	Envelope e;

	for (;;)
	{
		// Receive message.
		int32_t nrecv = m_peers->receive(&e, size + 3, outFromHandle);
		if (nrecv <= 0)
			return 0;

		m_state[outFromHandle].flags |= SfReceived;

		// Check destination.
		if (e.to == m_peers->getHandle())
		{
			std::memcpy(data, e.payload, nrecv - 3);
			outFromHandle = e.from;
			return nrecv - 3;
		}
		// Need to relay further.
		else
		{
			bool reliable = bool((e.flags & 0x80) == 0x80);
			if (!m_peers->send(e.to, &e, nrecv, reliable))
			{
				uint8_t count = e.flags & 0x7f;
				if (++count < 64)
				{
					e.flags = (e.flags & 0x80) | count;

					std::vector< uint8_t > peerHandles;
					for (std::vector< PeerInfo >::iterator i = m_info.begin(); i != m_info.end(); ++i)
					{
						if (i->handle != e.from && i->handle != e.to)
							peerHandles.push_back(i->handle);
						else if (i->handle == e.to)
							i->relayed = true;
					}

					std::random_shuffle(peerHandles.begin(), peerHandles.end());
					for (std::vector< uint8_t >::const_iterator i = peerHandles.begin(); i != peerHandles.end(); ++i)
					{
						if (m_peers->send(*i, &e, nrecv, reliable))
							break;
					}
				}
			}
		}
	}

	// Unreachable.
	return 0;
}

bool RelayPeers::send(handle_t handle, const void* data, int32_t size, bool reliable)
{
	// Create transport envelope.
	Envelope e;
	e.flags = (reliable ? 0x80 : 0x00);
	e.from = m_peers->getHandle();
	e.to = handle;
	std::memcpy(e.payload, data, size);

	// Send message; reverse order if already sent directly but nothing received.
	uint32_t flags = m_state[handle].flags & (SfSent | SfReceived);
	if (
		flags == 0 ||
		flags == (SfSent | SfReceived)
	)
	{
		m_state[handle].flags |= SfSent;

		if (sendDirect(e, size, reliable))
			return true;
		if (sendRelay(e, size, reliable))
			return true;

		m_state[handle].flags &= ~SfSent;
	}
	else
	{
		m_state[handle].flags |= SfSent;

		if (sendRelay(e, size, reliable))
			return true;
		if (sendDirect(e, size, reliable))
			return true;

		m_state[handle].flags &= ~SfSent;
	}

	return false;
}

bool RelayPeers::sendDirect(const Envelope& e, uint32_t payloadSize, bool reliable)
{
	if (!m_peers->send(e.to, &e, payloadSize + 3, reliable))
		return false;

	m_state[e.to].flags &= ~SfRelayed;
	return true;
}

bool RelayPeers::sendRelay(const Envelope& e, uint32_t payloadSize, bool reliable)
{
	std::vector< uint8_t > peerHandles;
	peerHandles.reserve(m_info.size());

	for (std::vector< PeerInfo >::iterator i = m_info.begin(); i != m_info.end(); ++i)
	{
		if (i->handle != e.from && i->handle != e.to)
			peerHandles.push_back(i->handle);
	}

	std::random_shuffle(peerHandles.begin(), peerHandles.end());
	for (std::vector< uint8_t >::const_iterator i = peerHandles.begin(); i != peerHandles.end(); ++i)
	{
		if (m_peers->send(*i, &e, payloadSize + 3, reliable))
		{
			m_state[e.to].flags |= SfRelayed;
			return true;
		}
	}

	m_state[e.to].flags &= ~SfRelayed;
	return false;
}

	}
}
