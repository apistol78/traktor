#include "Core/Io/MemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Misc/SafeDestroy.h"
#include "Parade/Network/CompactSerializer.h"
#include "Parade/Network/IReplicatableState.h"
#include "Parade/Network/IReplicatorPeers.h"
#include "Parade/Network/Message.h"
#include "Parade/Network/Replicator.h"
#include "Parade/Network/StateProphet.h"

namespace traktor
{
	namespace parade
	{
		namespace
		{

const handle_t c_broadcastHandle = 0UL;
const float c_maxLatencyCompensate = 2.0f;
const float c_maxOffsetAdjustError = 4.0f;
const float c_maxOffsetAdjust = 2.0f;
const float c_maxStateAge = 2.0f;
const float c_nearDistance = 28.0f;
const float c_farDistance = 250.0f;
const float c_nearTimeUntilTx = 1.0f / 20.0f;
const float c_farTimeUntilTx = 1.0f / 10.0f;
const float c_timeUntilIAm = 4.0f;

#define T_REPLICATOR_DEBUG(x) traktor::log::info << x << traktor::Endl

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.Replicator", Replicator, Object)

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.Replicator.IListener", Replicator::IListener, Object)

Replicator::Replicator()
:	m_id(Guid::create())
,	m_origin(0.0f, 0.0f, 0.0f, 1.0f)
,	m_time(0.0f)
{
}

Replicator::~Replicator()
{
	destroy();
}

bool Replicator::create(IReplicatorPeers* replicatorPeers)
{
	std::vector< handle_t > handles;

	m_replicatorPeers = replicatorPeers;
	m_state = 0;

	m_replicatorPeers->update();
	m_replicatorPeers->getPeerHandles(handles);

	// Discard any pending data.
	while (m_replicatorPeers->receiveAnyPending())
	{
		uint8_t discard[sizeof(Message)];
		handle_t fromHandle;

		if (m_replicatorPeers->receive(discard, sizeof(discard), fromHandle) > 0)
			T_REPLICATOR_DEBUG(L"OK: Pending message discarded from peer " << fromHandle);
	}

	// Create non-established entries for each peer.
	for (std::vector< handle_t >::const_iterator i = handles.begin(); i != handles.end(); ++i)
	{
		Peer& peer = m_peers[*i];
		peer.established = false;
	}

	return true;
}

void Replicator::destroy()
{
	for (std::map< handle_t, Peer >::iterator i = m_peers.begin(); i != m_peers.end(); ++i)
	{
		sendBye(i->first);
		if (i->second.ghost)
		{
			i->second.ghost->~Ghost();
			getAllocator()->free(i->second.ghost);
		}
	}

	m_peers.clear();
	m_eventsIn.clear();
	m_eventsOut.clear();
	m_listeners.clear();
	m_eventTypes.clear();

	m_state = 0;
	m_replicatorPeers = 0;
}

void Replicator::addEventType(const TypeInfo& eventType)
{
	m_eventTypes.push_back(&eventType);
}

void Replicator::addListener(IListener* listener)
{
	m_listeners.push_back(listener);
}

void Replicator::update(float dT)
{
	std::set< handle_t > unfresh;
	std::vector< handle_t > handles;
	handle_t handle;
	Message msg;

	// Massage replicator peers back-end first and
	// get fresh list of peer handles.
	m_replicatorPeers->update();
	m_replicatorPeers->getPeerHandles(handles);

	// Keep list of unfresh handles.
	for (std::map< handle_t, Peer >::iterator i = m_peers.begin(); i != m_peers.end(); ++i)
		unfresh.insert(i->first);

	// Iterate all handles, tag peers as fresh or send "I am" to new peers.
	for (std::vector< handle_t >::const_iterator i = handles.begin(); i != handles.end(); ++i)
	{
		Peer& peer = m_peers[*i];

		// Issue "I am" to unestablished peers.
		if (
			!peer.established &&
			!peer.disconnected
		)
		{
			if ((peer.timeUntilTx -= dT) <= 0.0f)
			{
				T_REPLICATOR_DEBUG(L"OK: Unestablished peer found; sending \"I am\" to peer " << *i);
				sendIAm(*i, 0, m_id);
				peer.timeUntilTx = c_timeUntilIAm;
			}
		}

		unfresh.erase(*i);
	}

	// Issue disconnect events to listeners.
	for (std::set< handle_t >::const_iterator i = unfresh.begin(); i != unfresh.end(); ++i)
	{
		std::map< handle_t, Peer >::iterator it = m_peers.find(*i);
		T_ASSERT (it != m_peers.end());

		Peer& peer = it->second;
		if (peer.established)
		{
			T_REPLICATOR_DEBUG(L"OK: Established peer " << *i << L" disconnected; issue listener event");
			T_ASSERT (peer.ghost);
			T_ASSERT (!peer.disconnected);

			// Need to notify listeners immediately as peer becomes dismounted.
			for (RefArray< IListener >::iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
				(*i)->notify(this, 0, IListener::ReDisconnected, it->first, 0);

			if (peer.ghost)
			{
				peer.ghost->~Ghost(); getAllocator()->free(peer.ghost);
				peer.ghost = 0;
			}

			peer.established = false;
			peer.disconnected = true;
		}

		// Remove unfresh peer.
		m_peers.erase(it);
	}

	// Read messages from any peer.
	while (m_replicatorPeers->receiveAnyPending())
	{
		if (m_replicatorPeers->receive(&msg, sizeof(msg), handle) < 0)
		{
			T_REPLICATOR_DEBUG(L"ERROR: Failed to receive pending message");
			continue;
		}

		// Convert time from ms to seconds.
		float time = msg.time / 1000.0f;

		// Always handle handshake messages.
		if (msg.type == MtIAm)
		{
			// Unwrap id and ensure it's valid.
			Guid id(msg.iam.id);
			if (!id.isNotNull())
			{
				T_REPLICATOR_DEBUG(L"ERROR: Corrupt sequence id received from peer " << handle << L"; ignoring response to handshake");
				continue;
			}

			T_REPLICATOR_DEBUG(L"OK: Got \"I am\" from peer " << handle << L", sequence " << int32_t(msg.iam.sequence) << L", id " << id.format());

			// Assume peer time is correct if exceeding my time.
			if (time > m_time)
				m_time = time;

			if (msg.iam.sequence == 0)
			{
				sendIAm(handle, 1, id);
			}
			else if (msg.iam.sequence == 1 || msg.iam.sequence == 2)
			{
				// "I am" with sequence 1 can only be received if I was the handshake initiator.
				// "I am" with sequence 2 can only be received if I was NOT the handshake initiator.

				if (msg.iam.sequence == 1)
				{
					if (id != m_id)
					{
						T_REPLICATOR_DEBUG(L"ERROR: \"I am\" message with incorrect id; ignoring");
						continue;
					}
					sendIAm(handle, 2, id);
				}

				Peer& peer = m_peers[handle];

				if (!peer.ghost)
				{
					// Create ghost data.
					void* ghostMem = getAllocator()->alloc(sizeof(Ghost), 16, "Ghost");

					peer.ghost = new (ghostMem) Ghost();
					peer.ghost->origin = m_origin;
					peer.ghost->prophet = new StateProphet();
				}

				if (!peer.established)
				{
					peer.established = true;
					peer.timeUntilTx = 0.0f;

					// Issue connect event to listeners.
					Event evt;
					evt.eventId = IListener::ReConnected;
					evt.handle = handle;
					evt.object = 0;
					m_eventsIn.push_back(evt);

					T_REPLICATOR_DEBUG(L"OK: Peer connection established");
				}
			}

			continue;
		}
		else if (msg.type == MtBye)
		{
			T_REPLICATOR_DEBUG(L"OK: Got \"Bye\" from peer");

			Peer& peer = m_peers[handle];

			if (peer.established && peer.ghost)
			{
				T_REPLICATOR_DEBUG(L"OK: Established peer gracefully disconnected; issue listener event");

				// Need to notify listeners immediately as peer becomes dismounted.
				for (RefArray< IListener >::iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
					(*i)->notify(this, 0, IListener::ReDisconnected, handle, 0);
			}

			if (peer.ghost)
			{
				peer.ghost->~Ghost(); getAllocator()->free(peer.ghost);
				peer.ghost = 0;
			}

			peer.established = false;
			peer.disconnected = true;
			continue;
		}

		// Not a handshake message.
		Peer& peer = m_peers[handle];
		if (!peer.ghost)
		{
			T_REPLICATOR_DEBUG(L"ERROR: Peer partially connected but received non-handshake message; ignoring");
			continue;
		}

		// Somehow the handshake wasn't fully successful but we've received a ghost then assume it's ok.
		if (!peer.established)
		{
			peer.established = true;
			peer.timeUntilTx = 0.0f;

			// Issue connect event to listeners.
			Event evt;
			evt.eventId = IListener::ReConnected;
			evt.handle = handle;
			evt.object = 0;
			m_eventsIn.push_back(evt);

			T_REPLICATOR_DEBUG(L"OK: Peer partially connected; but since we've got a ghost then we accept");
		}

		if (msg.type == MtState)	// Data message.
		{
			// Ignore old messages; as we're using unreliable transportation
			// messages can arrive out-of-order.
			if (time > peer.lastTime + 1e-8f)
			{
				// Adjust time based on network latency etc.
				if (time > m_time)
				{
					float offsetAdjust = time - m_time;
					if (offsetAdjust > c_maxOffsetAdjustError)
					{
						T_REPLICATOR_DEBUG(L"ERROR: Corrupt time (" << time << L") from peer " << handle << L"; package ignored");
						continue;
					}

					offsetAdjust = min(offsetAdjust, c_maxOffsetAdjust);
					float latencyAdjust = peer.packetCount > 0 ? min(offsetAdjust / 2.0f, c_maxLatencyCompensate) : 0.0f;

					T_REPLICATOR_DEBUG(L"OK: Adjusting time, offset " << int32_t(offsetAdjust * 1000.0f) << L", latency " << int32_t(latencyAdjust * 1000.0f) << L" ms");
					m_time += offsetAdjust + latencyAdjust;
				}
				// Check if message is too old.
				else if ((m_time - time) > c_maxStateAge)
				{
					T_REPLICATOR_DEBUG(L"WARNING: Too old package; package ignored");
					peer.lastTime = time;
					continue;
				}

				MemoryStream s(msg.data, sizeof(msg.data), true, false);
				Ref< IReplicatableState > state = CompactSerializer(&s, &m_eventTypes[0]).readObject< IReplicatableState >();
				if (state)
					peer.ghost->prophet->push(time, state);

				peer.lastTime = time;
				peer.packetCount++;
				peer.latency = lerp(peer.latency, m_time - time, 0.1f);

				// Put an input event to notify listeners about new state.
				Event evt;
				evt.eventId = IListener::ReState;
				evt.handle = handle;
				evt.object = 0;
				m_eventsIn.push_back(evt);
			}
			else
				T_REPLICATOR_DEBUG(L"OK: Too old package received from peer " << handle << L"; package ignored");
		}
		else if (msg.type == MtEvent)	// Event message.
		{
			MemoryStream s(msg.data, sizeof(msg.data), true, false);
			Ref< ISerializable > eventObject = CompactSerializer(&s, &m_eventTypes[0]).readObject< ISerializable >();

			// Put an input event to notify listeners about received event.
			Event e;
			e.time = time;
			e.eventId = IListener::ReBroadcastEvent;
			e.handle = handle;
			e.object = eventObject;
			m_eventsIn.push_back(e);
		}
	}

	// Send events to peers.
	if (!m_eventsOut.empty())
	{
		for (std::list< Event >::const_iterator i = m_eventsOut.begin(); i != m_eventsOut.end(); ++i)
		{
			msg.type = MtEvent;
			msg.time = uint32_t(m_time * 1000.0f);

			MemoryStream s(msg.data, sizeof(msg.data), false, true);
			CompactSerializer cs(&s, &m_eventTypes[0]);
			cs.writeObject(i->object);
			cs.flush();

			uint32_t msgSize = sizeof(uint8_t) + sizeof(float) + s.tell();

			if (i->handle == c_broadcastHandle)
			{
				for (std::map< handle_t, Peer >::const_iterator j = m_peers.begin(); j != m_peers.end(); ++j)
				{
					if (!j->second.established)
						continue;

					if (!m_replicatorPeers->send(j->first, &msg, msgSize, true))
						log::error << L"ERROR: Unable to send event to peer " << j->first << Endl;
				}
			}
			else
			{
				if (!m_replicatorPeers->send(i->handle, &msg, msgSize, true))
					log::error << L"ERROR: Unable to event to peer " << i->handle << Endl;
			}
		}
		m_eventsOut.clear();
	}

	// Broadcast my state to all peers.
	if (m_state)
	{
		msg.type = MtState;
		msg.time = uint32_t(m_time * 1000.0f);

		MemoryStream s(msg.data, sizeof(msg.data), false, true);
		CompactSerializer cs(&s, &m_eventTypes[0]);
		cs.writeObject(m_state);
		cs.flush();

		uint32_t msgSize = sizeof(uint8_t) + sizeof(float) + s.tell();

		for (std::map< handle_t, Peer >::iterator i = m_peers.begin(); i != m_peers.end(); ++i)
		{
			Peer& peer = i->second;

			if (!peer.established || !peer.ghost)
				continue;
			if ((peer.timeUntilTx -= dT) > 0.0f)
				continue;

			if (!m_replicatorPeers->sendReady(i->first))
				continue;

			if (!m_replicatorPeers->send(i->first, &msg, msgSize, false))
				log::error << L"ERROR: Unable to send state to peer " << i->first << Endl;

			float distanceToPeer = (peer.ghost->origin - m_origin).xyz0().length();
			float t = clamp((distanceToPeer - c_nearDistance) / (c_farDistance - c_nearDistance), 0.0f, 1.0f);
			
			peer.timeUntilTx = lerp(c_nearTimeUntilTx, c_farTimeUntilTx, t);
		}
	}

	// Dispatch event listeners.
	while (!m_eventsIn.empty())
	{
		const Event& event = m_eventsIn.front();
		for (RefArray< IListener >::iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
			(*i)->notify(this, event.time, event.eventId, event.handle, event.object);
		m_eventsIn.pop_front();
	}

	m_time += dT;
}

void Replicator::setOrigin(const Vector4& origin)
{
	m_origin = origin;
}

void Replicator::setState(const IReplicatableState* state)
{
	m_state = state;
}

void Replicator::sendEvent(handle_t peerHandle, const ISerializable* eventObject)
{
	Event e;
	e.time = 0.0f;
	e.eventId = 0;
	e.handle = peerHandle;
	e.object = eventObject;
	m_eventsOut.push_back(e);
}

void Replicator::broadcastEvent(const ISerializable* eventObject)
{
	Event e;
	e.time = 0.0f;
	e.eventId = 0;
	e.handle = c_broadcastHandle;
	e.object = eventObject;
	m_eventsOut.push_back(e);
}

bool Replicator::isPrimary() const
{
	return m_replicatorPeers->isPrimary();
}

uint32_t Replicator::getPeerCount() const
{
	return uint32_t(m_peers.size());
}

handle_t Replicator::getPeerHandle(uint32_t peerIndex) const
{
	std::map< handle_t, Peer >::const_iterator it = m_peers.begin();
	std::advance(it, peerIndex);
	return it->first;
}

std::wstring Replicator::getPeerName(handle_t peerHandle) const
{
	return m_replicatorPeers->getPeerName(peerHandle);
}

bool Replicator::isPeerConnected(handle_t peerHandle) const
{
	std::map< handle_t, Peer >::const_iterator i = m_peers.find(peerHandle);
	if (i == m_peers.end())
		return false;
	else if (!i->second.established || !i->second.ghost)
		return false;
	else
		return true;
}

void Replicator::setGhostObject(handle_t peerHandle, Object* ghostObject)
{
	std::map< handle_t, Peer >::iterator i = m_peers.find(peerHandle);
	if (i != m_peers.end() && i->second.ghost)
		i->second.ghost->object = ghostObject;
	else
		T_REPLICATOR_DEBUG(L"ERROR: Trying to set ghost object of unknown peer handle " << peerHandle);
}

Object* Replicator::getGhostObject(handle_t peerHandle) const
{
	std::map< handle_t, Peer >::const_iterator i = m_peers.find(peerHandle);
	if (i != m_peers.end() && i->second.ghost)
		return i->second.ghost->object;
	else
	{
		T_REPLICATOR_DEBUG(L"ERROR: Trying to get ghost object of unknown peer handle " << peerHandle);
		return 0;
	}
}

void Replicator::setGhostOrigin(handle_t peerHandle, const Vector4& origin)
{
	std::map< handle_t, Peer >::iterator i = m_peers.find(peerHandle);
	if (i != m_peers.end() && i->second.ghost)
		i->second.ghost->origin = origin;
	else
		T_REPLICATOR_DEBUG(L"ERROR: Trying to set ghost origin of unknown peer handle " << peerHandle);
}

Ref< const IReplicatableState > Replicator::getGhostState(handle_t peerHandle) const
{
	std::map< handle_t, Peer >::const_iterator i = m_peers.find(peerHandle);
	if (i != m_peers.end() && i->second.ghost)
		return i->second.ghost->prophet->get(m_time);
	else
	{
		T_REPLICATOR_DEBUG(L"ERROR: Trying to get ghost state of unknown peer handle " << peerHandle);
		return 0;
	}
}

void Replicator::sendIAm(handle_t peerHandle, uint8_t sequence, const Guid& id)
{
	Message msg;

	msg.type = MtIAm;
	msg.time = uint32_t(m_time * 1000.0f);
	msg.iam.sequence = sequence;

	std::memcpy(msg.iam.id, id, sizeof(msg.iam.id));

	uint32_t msgSize = sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint8_t) * (1 + 16);
	m_replicatorPeers->send(peerHandle, &msg, msgSize, true);
}

void Replicator::sendBye(handle_t peerHandle)
{
	Message msg;

	msg.type = MtBye;
	msg.time = uint32_t(m_time * 1000.0f);

	uint32_t msgSize = sizeof(uint8_t) + sizeof(uint32_t);
	m_replicatorPeers->send(peerHandle, &msg, msgSize, true);
}

	}
}
