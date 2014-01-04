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

#if 1
#	define T_RELAY_DEBUG(x) traktor::log::info << x << traktor::Endl
#else
#	define T_RELAY_DEBUG(x)
#endif

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.RelayPeers", RelayPeers, IReplicatorPeers)

RelayPeers::RelayPeers(IReplicatorPeers* peers)
:	m_peers(peers)
,	m_connectionState(0)
,	m_connectionStateLast(0)
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

bool RelayPeers::update()
{
	T_ASSERT (m_peers);

	if (!m_peers->update())
		return false;

	m_info.resize(0);
	m_peers->getPeers(m_info);

	for (std::vector< PeerInfo >::iterator i = m_info.begin(); i != m_info.end(); ++i)
		i->direct = bool((m_connectionState & (1ULL << i->handle)) != 0);

	// Expose our connection state bitmask.
	if (m_connectionStateLast != m_connectionState)
	{
		m_peers->setConnectionState(m_connectionState | 1);
		m_connectionStateLast = m_connectionState;
	}

	return true;
}

void RelayPeers::setStatus(uint8_t status)
{
	m_peers->setStatus(status);
}

void RelayPeers::setConnectionState(uint64_t connectionState)
{
	m_peers->setConnectionState(connectionState);
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
			if (!sendDirect(e, nrecv - 3, reliable))
			{
				uint8_t count = e.flags & 0x7f;
				if (++count < 64)
				{
					e.flags = (e.flags & 0x80) | count;
					sendRelay(e, nrecv - 3, reliable);
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

	// Send message, always try direct before relaying.
	if (sendDirect(e, size, reliable))
		return true;
	if (sendRelay(e, size, reliable))
		return true;

	return false;
}

bool RelayPeers::sendDirect(const Envelope& e, uint32_t payloadSize, bool reliable)
{
	if (m_peers->send(e.to, &e, payloadSize + 3, reliable))
	{
		m_connectionState |= (1ULL << e.to);
		return true;
	}
	else
	{
		m_connectionState &= ~(1ULL << e.to);
		return false;
	}
}

bool RelayPeers::sendRelay(const Envelope& e, uint32_t payloadSize, bool reliable)
{
	State& state = m_state[e.to];

	m_connectionState &= ~(1ULL << e.to);

	// Prefer relaying through same peer as last time.
	if (state.relayer)
	{
		// Ensure relayer peer still is direct connected.
		if ((m_connectionState & (1ULL << state.relayer)) != 0)
		{
			if (m_peers->send(state.relayer, &e, payloadSize + 3, reliable))
				return true;
			else
				m_connectionState &= ~(1ULL << state.relayer);
		}
		else
			state.relayer = 0;
	}

	// Collect relayer peer candidates.
	std::vector< uint8_t > peerHandles;
	peerHandles.reserve(m_info.size());

	for (std::vector< PeerInfo >::iterator i = m_info.begin(); i != m_info.end(); ++i)
	{
		if (
			i->handle != e.from &&
			i->handle != e.to &&
			(m_connectionState & (1ULL << i->handle)) != 0 &&	// Are we able to send to peer?
			(i->connectionState & (1ULL << e.to)) != 0			// Are peer able to send to target?
		)
			peerHandles.push_back(i->handle);
	}

	if (peerHandles.empty())
	{
		for (std::vector< PeerInfo >::iterator i = m_info.begin(); i != m_info.end(); ++i)
		{
			if (
				i->handle != e.from &&
				i->handle != e.to &&
				(m_connectionState & (1ULL << i->handle)) != 0	// Are we able to send to peer?
			)
				peerHandles.push_back(i->handle);
		}
	}

	// Randomize and then try each one in turn, pick first successful.
	std::random_shuffle(peerHandles.begin(), peerHandles.end());
	for (std::vector< uint8_t >::const_iterator i = peerHandles.begin(); i != peerHandles.end(); ++i)
	{
		if (m_peers->send(*i, &e, payloadSize + 3, reliable))
		{
			state.relayer = *i;
			return true;
		}
		else
			m_connectionState &= ~(1ULL << *i);
	}

	T_ASSERT (state.relayer == 0);
	return false;
}

	}
}
