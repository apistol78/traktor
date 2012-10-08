#include "Core/Io/MemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Misc/SafeDestroy.h"
#include "Parade/Network/CompactSerializer.h"
#include "Parade/Network/IReplicatorPeers.h"
#include "Parade/Network/Message.h"
#include "Parade/Network/Replicator.h"
#include "Parade/Network/State/StateTemplate.h"

namespace traktor
{
	namespace parade
	{
		namespace
		{

const handle_t c_broadcastHandle = 0UL;
const float c_maxOffsetAdjust = 2.0f;
const float c_nearDistance = 30.0f;
const float c_farDistance = 250.0f;
const float c_nearTimeUntilTx = 1.0f / 20.0f;
const float c_farTimeUntilTx = 1.0f / 10.0f;
const float c_timeUntilIAm = 6.0f;
const float c_timeUntilPing = 1.0f;
const float c_peerTimeout = 20.0f;
const float c_distanceBlend = 0.75f;
const uint32_t c_maxPendingPing = 16;

#define T_REPLICATOR_DEBUG(x) traktor::log::info << x << traktor::Endl

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.Replicator", Replicator, Object)

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.Replicator.IListener", Replicator::IListener, Object)

Replicator::Replicator()
:	m_id(Guid::create())
,	m_origin(0.0f, 0.0f, 0.0f, 1.0f)
,	m_time(0.0f)
,	m_pingCount(0)
,	m_timeUntilPing(0.0f)
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

		if (peer.disconnected)
			continue;

		// Issue "I am" to unestablished peers.
		if (!peer.established)
		{
			if ((peer.timeUntilTx -= dT) <= 0.0f)
			{
				T_REPLICATOR_DEBUG(L"OK: Unestablished peer found; sending \"I am\" to peer " << *i);
				sendIAm(*i, 0, m_id);
				peer.timeUntilTx = c_timeUntilIAm;
			}
		}

		// Check if peer doesn't respond or timeout;ed.
		if (
			peer.established &&
			peer.packetCount > 0
		)
		{
			T_ASSERT (peer.lastTime > 0.0f);

			bool failing = false;

			float T = m_time - peer.lastTime;
			if (T > c_peerTimeout)
			{
				T_REPLICATOR_DEBUG(L"WARNING: Peer " << *i << L" timeout, no packet in " << int32_t(T * 1000.0f) << L" ms");
				failing = true;
			}

			if (peer.pendingPing > c_maxPendingPing)
			{
				T_REPLICATOR_DEBUG(L"WARNING: Peer " << *i << L" doesn't respond to ping");
				failing = true;
			}

			if (failing)
			{
				// Need to notify listeners immediately as peer becomes dismounted.
				for (RefArray< IListener >::iterator j = m_listeners.begin(); j != m_listeners.end(); ++j)
					(*j)->notify(this, 0, IListener::ReDisconnected, *i, 0);

				if (peer.ghost)
				{
					peer.ghost->~Ghost(); getAllocator()->free(peer.ghost);
					peer.ghost = 0;
				}

				peer.established = false;
				peer.disconnected = true;
				continue;
			}
		}

		// Remove from "unfresh" peers.
		unfresh.erase(*i);
	}

	// Remove unfresh peers.
	for (std::set< handle_t >::const_iterator i = unfresh.begin(); i != unfresh.end(); ++i)
	{
		std::map< handle_t, Peer >::iterator it = m_peers.find(*i);
		T_ASSERT (it != m_peers.end());

		Peer& peer = it->second;
		if (peer.established && !peer.disconnected)
			continue;

		m_peers.erase(it);
	}

	// Broadcast my state to all peers.
	if (m_state && m_stateTemplate)
	{
		msg.type = MtState;
		msg.time = uint32_t(m_time * 1000.0f);

		uint32_t ss = m_stateTemplate->pack(
			m_state,
			msg.data,
			sizeof(msg.data)
		);

		uint32_t msgSize = sizeof(uint8_t) + sizeof(float) + ss;
		std::vector< std::pair< handle_t, Peer* > > peers;

		// Collect peers to which we should send state to.
		for (std::map< handle_t, Peer >::iterator i = m_peers.begin(); i != m_peers.end(); ++i)
		{
			Peer& peer = i->second;

			if (!peer.established || !peer.ghost)
				continue;
			if ((peer.timeUntilTx -= dT) > 0.0f)
				continue;

			if (!m_replicatorPeers->sendReady(i->first))
				continue;

			peers.push_back(std::make_pair(i->first, &peer));
		}

		// Randomize peers to prevent same send order multiple frames.
		std::random_shuffle(peers.begin(), peers.end());

		// Send state to peers.
		for (std::vector< std::pair< handle_t, Peer* > >::iterator i = peers.begin(); i != peers.end(); ++i)
		{
			Peer& peer = *i->second;
			if (m_replicatorPeers->send(i->first, &msg, msgSize, false))
			{
				float distanceToPeer = (peer.ghost->origin - m_origin).xyz0().length();
				float t0 = clamp((distanceToPeer - c_nearDistance) / (c_farDistance - c_nearDistance), 0.0f, 1.0f);
				float t1 = std::sqrt(t0);
				float t = lerp(t0, t1, c_distanceBlend);
				peer.timeUntilTx = lerp(c_nearTimeUntilTx, c_farTimeUntilTx, t);
			}
			else
			{
				log::error << L"ERROR: Unable to send state to peer " << i->first << Endl;
				peer.timeUntilTx = c_farTimeUntilTx;
			}
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

	// Send ping to peers.
	m_timeUntilPing -= dT;
	if (m_timeUntilPing <= 0.0f)
	{
		if (!m_peers.empty())
		{
			std::map< handle_t, Peer >::iterator i = m_peers.begin();

			// Ping one peer at a time.
			m_pingCount = (m_pingCount + 1) % m_peers.size();
			std::advance(i, m_pingCount);

			Peer& peer = i->second;
			if (peer.established)
			{
				sendPing(i->first);
				++peer.pendingPing;
			}

			m_timeUntilPing = c_timeUntilPing / m_peers.size();
		}
		else
			m_timeUntilPing = c_timeUntilPing;
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
						T_REPLICATOR_DEBUG(L"ERROR: \"I am\" message with incorrect id from peer " << handle << L"; ignoring");
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
					peer.ghost->Tn2 = 0.0f;
					peer.ghost->Tn1 = 0.0f;
					peer.ghost->T0 = 0.0f;
				}

				if (!peer.established)
				{
					peer.established = true;
					peer.timeUntilTx = 0.0f;

					// Send ping to peer.
					sendPing(handle);

					// Issue connect event to listeners.
					Event evt;
					evt.eventId = IListener::ReConnected;
					evt.handle = handle;
					evt.object = 0;
					m_eventsIn.push_back(evt);

					T_REPLICATOR_DEBUG(L"OK: Peer " << handle << L" connection established");
				}

				if (time > peer.lastTime + 1e-4f)
					peer.lastTime = time;
			}
		}
		else if (msg.type == MtBye)
		{
			T_REPLICATOR_DEBUG(L"OK: Got \"Bye\" from peer " << handle);

			Peer& peer = m_peers[handle];

			if (peer.established && peer.ghost)
			{
				T_REPLICATOR_DEBUG(L"OK: Established peer " << handle << L" gracefully disconnected; issue listener event");

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

			if (time > peer.lastTime + 1e-4f)
				peer.lastTime = time;
		}
		else if (msg.type == MtPing)
		{
			// I've got pinged; reply with a pong.
			Peer& peer = m_peers[handle];
			peer.lastTime = time;

			sendPong(handle, msg.time);
		}
		else if (msg.type == MtPong)
		{
			// I've received a pong reply from an earlier ping;
			// calculate round-trip time.

			Peer& peer = m_peers[handle];
			if (peer.pendingPing > 0)
			{
				float pingTime = float(msg.pong.time0 / 1000.0f);
				float pongTime = float(msg.time / 1000.0f);
				float roundTrip = max(m_time - pingTime, 0.0f);

				peer.roundTrips.push_back(roundTrip);

				// Get lowest and median round-trips and calculate latencies.
				float minrt = roundTrip;
				float sorted[MaxRoundTrips];
				for (uint32_t i = 0; i < peer.roundTrips.size(); ++i)
				{
					sorted[i] = peer.roundTrips[i];
					minrt = min(minrt, peer.roundTrips[i]);
				}

				std::sort(&sorted[0], &sorted[peer.roundTrips.size()]);

				peer.latencyMedian = sorted[peer.roundTrips.size() / 2] / 2.0f;
				peer.latencyMinimum = minrt / 2.0f;
				peer.latencyReversed = float(msg.pong.latency / 1000.0f);

				if (time > peer.lastTime + 1e-4f)
					peer.lastTime = time;

				--peer.pendingPing;
			}
			else
				T_REPLICATOR_DEBUG(L"ERROR: Got pong response from peer " << handle << L" but no ping sent; ignored");
		}
		else if (msg.type == MtState)	// Data message.
		{
			Peer& peer = m_peers[handle];
			if (!peer.ghost)
			{
				T_REPLICATOR_DEBUG(L"ERROR: Peer " << handle << L" partially connected but received MtState; ignoring");
				continue;
			}

			// Ignore old messages; as we're using unreliable transportation
			// messages can arrive out-of-order.
			if (time > peer.lastTime + 1e-4f)
			{
				peer.lastTime = time;

				if (time + peer.latencyMinimum > m_time + 1e-5f)
				{
					// Adjust time; adjust only with 75% of the difference in order
					// to stabilize synchronization over multiple iterations.
					float offset = time + peer.latencyMinimum - m_time;
					float adjust = min(offset * 0.75f, c_maxOffsetAdjust);
					m_time += adjust;
				}

				if (peer.ghost->stateTemplate)
				{
					Ref< State > state = peer.ghost->stateTemplate->unpack(msg.data, sizeof(msg.data));
					if (state)
					{
						peer.ghost->Sn2 = peer.ghost->Sn1;
						peer.ghost->Tn2 = peer.ghost->Tn1;
						peer.ghost->Sn1 = peer.ghost->S0;
						peer.ghost->Tn1 = peer.ghost->T0;
						peer.ghost->S0 = state;
						peer.ghost->T0 = time;
					}
					else
						T_REPLICATOR_DEBUG(L"ERROR: Unable to unpack ghost state");
				}
				else
					T_REPLICATOR_DEBUG(L"WARNING: Ghost state received but no state template associated with ghost, thus unable to decode state");

				peer.packetCount++;

				// Put an input event to notify listeners about new state.
				Event evt;
				evt.eventId = IListener::ReState;
				evt.handle = handle;
				evt.object = 0;
				m_eventsIn.push_back(evt);
			}
			else
			{
				// Received an old out-of-order package.
				T_REPLICATOR_DEBUG(L"WARNING: Out of order package received; ignored");
				peer.packetCount++;
			}
		}
		else if (msg.type == MtEvent)	// Event message.
		{
			Peer& peer = m_peers[handle];
			if (!peer.ghost)
			{
				T_REPLICATOR_DEBUG(L"ERROR: Peer " << handle << L" partially connected but received MtEvent; ignoring");
				continue;
			}

			if (time > peer.lastTime + 1e-4f)
				peer.lastTime = time;

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

	// Dispatch event listeners.
	for (std::list< Event >::const_iterator i = m_eventsIn.begin(); i != m_eventsIn.end(); ++i)
	{
		const Event& event = *i;
		for (RefArray< IListener >::iterator j = m_listeners.begin(); j != m_listeners.end(); ++j)
			(*j)->notify(this, event.time, event.eventId, event.handle, event.object);
	}
	m_eventsIn.clear();

	m_time += dT;
}

void Replicator::setOrigin(const Vector4& origin)
{
	m_origin = origin;
}

void Replicator::setStateTemplate(const StateTemplate* stateTemplate)
{
	m_stateTemplate = stateTemplate;
}

void Replicator::setState(const State* state)
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

int32_t Replicator::getPeerLatency(handle_t peerHandle) const
{
	std::map< handle_t, Peer >::const_iterator i = m_peers.find(peerHandle);
	return i != m_peers.end() ? int32_t(i->second.latencyMinimum * 1000.0f) : 0;
}

int32_t Replicator::getPeerReversedLatency(handle_t peerHandle) const
{
	std::map< handle_t, Peer >::const_iterator i = m_peers.find(peerHandle);
	return i != m_peers.end() ? int32_t(i->second.latencyReversed * 1000.0f) : 0;
}

int32_t Replicator::getWorstReversedLatency() const
{
	int32_t latencyWorst = 0;
	for (std::map< handle_t, Peer >::const_iterator i = m_peers.begin(); i != m_peers.end(); ++i)
		latencyWorst = std::max(latencyWorst, int32_t(i->second.latencyReversed * 1000.0f));
	return latencyWorst;
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

void Replicator::setGhostStateTemplate(handle_t peerHandle, const StateTemplate* stateTemplate)
{
	std::map< handle_t, Peer >::iterator i = m_peers.find(peerHandle);
	if (i != m_peers.end() && i->second.ghost)
		i->second.ghost->stateTemplate = stateTemplate;
	else
		T_REPLICATOR_DEBUG(L"ERROR: Trying to get ghost state of unknown peer handle " << peerHandle);
}

Ref< const State > Replicator::getGhostState(handle_t peerHandle) const
{
	std::map< handle_t, Peer >::const_iterator i = m_peers.find(peerHandle);
	if (i != m_peers.end() && i->second.ghost)
	{
		const StateTemplate* stateTemplate = i->second.ghost->stateTemplate;
		if (stateTemplate)
			return stateTemplate->extrapolate(
				i->second.ghost->Sn2,
				i->second.ghost->Tn2,
				i->second.ghost->Sn1,
				i->second.ghost->Tn1,
				i->second.ghost->S0,
				i->second.ghost->T0,
				m_time
			);
		else
			return i->second.ghost->S0;
	}
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

void Replicator::sendPing(handle_t peerHandle)
{
	Message msg;

	msg.type = MtPing;
	msg.time = uint32_t(m_time * 1000.0f);

	uint32_t msgSize = sizeof(uint8_t) + sizeof(uint32_t);
	m_replicatorPeers->send(peerHandle, &msg, msgSize, false);
}

void Replicator::sendPong(handle_t peerHandle, uint32_t time0)
{
	std::map< handle_t, Peer >::const_iterator i = m_peers.find(peerHandle);

	Message msg;

	msg.type = MtPong;
	msg.time = uint32_t(m_time * 1000.0f);
	msg.pong.time0 = time0;
	msg.pong.latency = (i != m_peers.end()) ? uint32_t(i->second.latencyMinimum * 1000.0f) : 0;	// Report back my perception of latency to this peer. 

	uint32_t msgSize = sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t);
	m_replicatorPeers->send(peerHandle, &msg, msgSize, false);
}

	}
}
