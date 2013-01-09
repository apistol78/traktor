#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Net/Replication/ReliableTransportPeers.h"

namespace traktor
{
	namespace net
	{
		namespace
		{

const double c_resendTime = 2.0f;	//< Resend reliable message after N seconds.
const double c_discardTime = 10.0f;	//< Discard reliable message after N seconds.
const uint32_t c_windowSize = 100;	//< Number of reliable messages kept in sent queue.

#define T_REPLICATOR_DEBUG(x) traktor::log::info << x << traktor::Endl

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.ReliableTransportPeers", ReliableTransportPeers, IReplicatorPeers)

ReliableTransportPeers::ReliableTransportPeers(IReplicatorPeers* peers)
:	m_peers(peers)
{
	m_timer.start();
}

ReliableTransportPeers::~ReliableTransportPeers()
{
	destroy();
}

void ReliableTransportPeers::destroy()
{
	safeDestroy(m_peers);
}

void ReliableTransportPeers::update()
{
	m_peers->update();

	// Get available peers.
	std::vector< handle_t > handles;
	m_peers->getPeerHandles(handles);

	// Add or remove control entries.
	for (std::map< handle_t, Control >::iterator i = m_control.begin(); i != m_control.end(); ++i)
		i->second.alive = false;
	for (std::vector< handle_t >::const_iterator i = handles.begin(); i != handles.end(); ++i)
		m_control[*i].alive = true;
	for (std::map< handle_t, Control >::iterator i = m_control.begin(); i != m_control.end(); )
	{
		if (!i->second.alive)
			m_control.erase(i++);
		else
			++i;
	}

	// Check if we need to resend or discard some reliable messages.
	double time = m_timer.getElapsedTime();
	for (std::map< handle_t, Control >::iterator i = m_control.begin(); i != m_control.end(); ++i)
	{
		if (i->second.sent.size() >= c_windowSize)
		{
			T_REPLICATOR_DEBUG(L"Too many messages queued in sent buffer");
			while (i->second.sent.size() >= c_windowSize)
				i->second.sent.pop_front();
		}

		for (std::list< ControlEnvelope >::iterator j = i->second.sent.begin(); j != i->second.sent.end(); )
		{
			if ((time - j->time0) >= c_discardTime)
			{
				T_REPLICATOR_DEBUG(L"No response from peer " << i->first << L" in " << c_discardTime << L" second(s); message discarded");
				j = i->second.sent.erase(j);
				continue;
			}
			if ((time - j->time) >= c_resendTime)
			{
				T_REPLICATOR_DEBUG(L"No response from peer " << i->first << L" in " << c_resendTime << L" second(s); message resent");
				m_peers->send(
					i->first,
					&j->envelope,
					j->size,
					false
				);
				j->time = time;
			}
			++j;
		}
	}
}

std::wstring ReliableTransportPeers::getName() const
{
	return m_peers->getName();
}

uint64_t ReliableTransportPeers::getGlobalId() const
{
	return m_peers->getGlobalId();
}

bool ReliableTransportPeers::isPrimary() const
{
	return m_peers->isPrimary();
}

uint32_t ReliableTransportPeers::getPeerHandles(std::vector< handle_t >& outPeerHandles) const
{
	outPeerHandles.resize(0);
	for (std::map< handle_t, Control >::const_iterator i = m_control.begin(); i != m_control.end(); ++i)
		outPeerHandles.push_back(i->first);
	return outPeerHandles.size();
}

std::wstring ReliableTransportPeers::getPeerName(handle_t handle) const
{
	return m_peers->getPeerName(handle);
}

uint64_t ReliableTransportPeers::getPeerGlobalId(handle_t handle) const
{
	return m_peers->getPeerGlobalId(handle);
}

int32_t ReliableTransportPeers::receive(void* data, int32_t size, handle_t& outFromHandle)
{
	Envelope e;

	for (;;)
	{
		// Receive message.
		int32_t nrecv = m_peers->receive(&e, size + 2, outFromHandle);
		if (nrecv <= 0)
			return 0;

		Control& ct = m_control[outFromHandle];

		// Send back ACK if reliable message.
		if (e.type == EtReliable)
		{
			Envelope ack;
			ack.type = EtAck;
			ack.sequence = e.sequence;
			m_peers->send(outFromHandle, &ack, 2, false);

			// We've already received this message.
			if (e.sequence == ct.last1_0 || e.sequence == ct.last1_1)
				continue;

			ct.last1_0 = ct.last1_1;
			ct.last1_1 = e.sequence;
		}

		// If unreliable message then we just check for duplicate messages.
		else if (e.type == EtUnreliable)
		{
			// We've already received this message.
			if (e.sequence == ct.last0_0 || e.sequence == ct.last0_1)
				continue;

			ct.last0_0 = ct.last0_1;
			ct.last0_1 = e.sequence;
		}

		// Did we receive an ACK then remove message from sent queue.
		else if (e.type == EtAck)
		{
			for (std::list< ControlEnvelope >::iterator i = ct.sent.begin(); i != ct.sent.end(); ++i)
			{
				if (i->envelope.sequence == e.sequence)
				{
					ct.sent.erase(i);
					break;
				}
			}
			continue;
		}

		std::memcpy(data, e.payload, nrecv - 2);
		return nrecv - 2;
	}

	// Unreachable.
	return 0;
}

bool ReliableTransportPeers::send(handle_t handle, const void* data, int32_t size, bool reliable)
{
	Control& ct = m_control[handle];

	// Create transport envelope.
	Envelope e;
	e.type = reliable ? EtReliable : EtUnreliable;
	e.sequence = 0x00;
	std::memcpy(e.payload, data, size);

	// Send message.
	if (!reliable)
	{
		e.sequence = ct.sequence0;
		if (!m_peers->send(handle, &e, 2 + size, false))
			return false;

		ct.sequence0++;
	}
	else
	{
		e.sequence = ct.sequence1;
		if (!m_peers->send(handle, &e, 2 + size, false))
			return false;

		ct.sequence1++;

		ControlEnvelope ce;
		ce.time0 =
		ce.time = m_timer.getElapsedTime();
		ce.size = 2 + size;
		ce.envelope = e;
		ct.sent.push_back(ce);
	}

	return true;
}

	}
}
