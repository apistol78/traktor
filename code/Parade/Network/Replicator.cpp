#include "Core/Io/MemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Parade/Network/CompactSerializer.h"
#include "Parade/Network/IReplicatableState.h"
#include "Parade/Network/IReplicatorPeers.h"
#include "Parade/Network/Message.h"
#include "Parade/Network/MessageRecorder.h"
#include "Parade/Network/Replicator.h"
#include "Parade/Network/StateProphet.h"

namespace traktor
{
	namespace parade
	{
		namespace
		{

const uint32_t c_broadcastPeerId = ~0UL;
const float c_maxLatencyCompensate = 2.0f;
const float c_maxOffsetAdjustError = 1000.0f;
const float c_maxOffsetAdjust = 30.0f;
const float c_nearDistance = 30.0f;
const float c_farDistance = 400.0f;
const float c_nearTimeUntilTx = 1.0f / 15.0f;
const float c_farTimeUntilTx = 1.0f / 2.0f;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.Replicator", Replicator, Object)

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.Replicator.IListener", Replicator::IListener, Object)

Replicator::Replicator()
:	m_origin(0.0f, 0.0f, 0.0f, 1.0f)
,	m_time(0.0f)
{
}

Replicator::~Replicator()
{
	destroy();
}

bool Replicator::create(IReplicatorPeers* replicatorPeers, const ISerializable* joinParams)
{
#if defined(_DEBUG)
	m_recorder = new MessageRecorder();
	if (!m_recorder->create())
		m_recorder = 0;
#endif

	m_replicatorPeers = replicatorPeers;
	m_joinParams = joinParams;

	// Send "I am" messages to all peers.
	uint32_t peerCount = m_replicatorPeers->getPeerCount();
	for (uint32_t i = 0; i < peerCount; ++i)
		sendIAm(i);

	return true;
}

void Replicator::destroy()
{
	safeDestroy(m_recorder);

	for (std::map< uint32_t, GhostPeer* >::iterator i = m_ghostPeers.begin(); i != m_ghostPeers.end(); ++i)
	{
		if (i->second)
			getAllocator()->free(i->second);
	}

	m_ghostPeers.clear();
	m_listeners.clear();
	m_state = 0;

	m_eventTypes.clear();
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
	uint32_t peerId;
	Message msg;

	// Read messages from any other peer.
	while (m_replicatorPeers->receiveAnyPending())
	{
		if (m_replicatorPeers->receive(&msg, sizeof(msg), peerId))
		{
			if (m_recorder)
				m_recorder->addMessage(m_time, peerId, msg);

			if (!m_ghostPeers[peerId])
				sendIAm(peerId);

			// Convert time from ms to seconds.
			float time = msg.time / 1000.0f;

			if (msg.type == MtIAm)	// "I am" message.
			{
				if (!m_ghostPeers[peerId])
				{
					// Unwrap "I am" parameters.
					MemoryStream s(msg.data, sizeof(msg.data), true, false);
					Ref< ISerializable > iAmParams = BinarySerializer(&s).readObject< ISerializable >();

					// Assume peer time is correct if exceeding my time.
					if (time > m_time)
					{
						T_DEBUG(L"Peer " << peerId << L" time exceeding ours; adjusting ours");
						m_time = time;
					}

					// Define ghost struct for this peer; must be 16 byte aligned thus placement new.
					void* ghostPeerMem = getAllocator()->alloc(sizeof(GhostPeer), 16, "GhostPeer");
					GhostPeer* ghostPeer = new (ghostPeerMem) GhostPeer();
					ghostPeer->origin = m_origin;
					ghostPeer->prophet = new StateProphet();
					ghostPeer->timeUntilTx = 0.0f;
					ghostPeer->lastTime = time;
					ghostPeer->packetCount = 0;
					ghostPeer->latency = 0.0f;
					m_ghostPeers[peerId] = ghostPeer;

					// Put an input event to notify listeners about new peer.
					Event evt;
					evt.eventId = IListener::ReConnected;
					evt.peerId = peerId;
					evt.object = iAmParams;
					m_eventsIn.push_back(evt);
				}
			}
			else if (msg.type == MtState)	// Data message.
			{
				GhostPeer* ghostPeer = m_ghostPeers[peerId];
				if (ghostPeer)
				{
					// Ignore old messages; as we're using unreliable transportation
					// messages can arrive out-of-order.
					if (time > ghostPeer->lastTime + 1e-8f)
					{
						// Adjust time based on network latency etc.
						if (time > m_time)
						{
							float offsetAdjust = time - m_time;
							if (offsetAdjust > c_maxOffsetAdjustError)
							{
								log::error << L"Corrupt time (" << time << L") from peer " << peerId << L"; package ignored" << Endl;
								continue;
							}

							offsetAdjust = min(offsetAdjust, c_maxOffsetAdjust);
							float latencyAdjust = min(offsetAdjust / 2.0f, c_maxLatencyCompensate);

							m_time += offsetAdjust + latencyAdjust;

							T_DEBUG(L"Peer " << peerId << L" time exceeding ours; adjusted with " << (offsetAdjust + latencyAdjust) * 1000.0f << L" ms");
						}

						MemoryStream s(msg.data, sizeof(msg.data), true, false);
						Ref< IReplicatableState > state = CompactSerializer(&s, &m_eventTypes[0]).readObject< IReplicatableState >();
						if (state)
							ghostPeer->prophet->push(time, state);

						ghostPeer->lastTime = time;
						ghostPeer->packetCount++;
						ghostPeer->latency = lerp(ghostPeer->latency, m_time - time, 0.1f);

						// Put an input event to notify listeners about new state.
						Event evt;
						evt.eventId = IListener::ReState;
						evt.peerId = peerId;
						evt.object = 0;
						m_eventsIn.push_back(evt);
					}
					else
						log::warning << L"Too old package received from peer " << peerId << L"; package ignored" << Endl;
				}
			}
			else if (msg.type == MtEvent)	// Event message.
			{
				GhostPeer* ghostPeer = m_ghostPeers[peerId];
				if (ghostPeer)
				{
					MemoryStream s(msg.data, sizeof(msg.data), true, false);
					Ref< ISerializable > eventObject = CompactSerializer(&s, &m_eventTypes[0]).readObject< ISerializable >();

					// Put an input event to notify listeners about received event.
					Event e;
					e.time = time;
					e.eventId = IListener::ReBroadcastEvent;
					e.peerId = peerId;
					e.object = eventObject;
					m_eventsIn.push_back(e);
				}
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

			if (i->peerId == c_broadcastPeerId)
			{
				for (std::map< uint32_t, GhostPeer* >::const_iterator j = m_ghostPeers.begin(); j != m_ghostPeers.end(); ++j)
				{
					if (!m_replicatorPeers->send(j->first, &msg, msgSize, true))
						log::error << L"Unable to event to peer " << peerId << Endl;
				}
			}
			else
			{
				std::map< uint32_t, GhostPeer* >::const_iterator j = m_ghostPeers.find(i->peerId);
				if (j != m_ghostPeers.end())
				{
					if (!m_replicatorPeers->send(j->first, &msg, msgSize, true))
						log::error << L"Unable to event to peer " << j->first << Endl;
				}
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

		for (std::map< uint32_t, GhostPeer* >::iterator i = m_ghostPeers.begin(); i != m_ghostPeers.end(); ++i)
		{
			GhostPeer* ghostPeer = i->second;
			if (!ghostPeer)
				continue;

			if ((ghostPeer->timeUntilTx -= dT) > 0.0f)
				continue;

			if (!m_replicatorPeers->sendReady(i->first))
				continue;

			if (!m_replicatorPeers->send(i->first, &msg, msgSize, false))
				log::error << L"Unable to send state to peer " << i->first << Endl;

			float distanceToPeer = (ghostPeer->origin - m_origin).xyz0().length();
			float t = clamp((distanceToPeer - c_nearDistance) / (c_farDistance - c_nearDistance), 0.0f, 1.0f);
			i->second->timeUntilTx = lerp(c_nearTimeUntilTx, c_farTimeUntilTx, t);
		}
	}

	// Dispatch event listeners.
	while (!m_eventsIn.empty())
	{
		const Event& event = m_eventsIn.front();
		for (RefArray< IListener >::iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
			(*i)->notify(this, event.time, event.eventId, event.peerId, event.object);
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

void Replicator::sendEvent(uint32_t peerId, const ISerializable* eventObject)
{
	Event e;
	e.eventId = 0;
	e.peerId = peerId;
	e.object = eventObject;
	m_eventsOut.push_back(e);
}

void Replicator::broadcastEvent(const ISerializable* eventObject)
{
	Event e;
	e.eventId = 0;
	e.peerId = c_broadcastPeerId;
	e.object = eventObject;
	m_eventsOut.push_back(e);
}

uint32_t Replicator::getMaxPeerId() const
{
	return m_replicatorPeers->getPeerCount();
}

void Replicator::setGhostObject(uint32_t peerId, Object* ghostObject)
{
	std::map< uint32_t, GhostPeer* >::iterator i = m_ghostPeers.find(peerId);
	if (i != m_ghostPeers.end())
		i->second->object = ghostObject;
}

Object* Replicator::getGhostObject(uint32_t peerId) const
{
	std::map< uint32_t, GhostPeer* >::const_iterator i = m_ghostPeers.find(peerId);
	return i != m_ghostPeers.end() ? i->second->object : 0;
}

void Replicator::setGhostOrigin(uint32_t peerId, const Vector4& origin)
{
	std::map< uint32_t, GhostPeer* >::iterator i = m_ghostPeers.find(peerId);
	if (i != m_ghostPeers.end())
		i->second->origin = origin;
}

Ref< const IReplicatableState > Replicator::getGhostState(uint32_t peerId) const
{
	std::map< uint32_t, GhostPeer* >::const_iterator i = m_ghostPeers.find(peerId);
	if (i != m_ghostPeers.end())
		return i->second->prophet->get(m_time);
	else
		return 0;
}

void Replicator::sendIAm(uint32_t peerId)
{
	Message msg;

	msg.type = MtIAm;
	msg.time = uint32_t(m_time * 1000.0f);

	MemoryStream s(msg.data, sizeof(msg.data), false, true);
	BinarySerializer(&s).writeObject(m_joinParams);

	uint32_t msgSize = sizeof(uint8_t) + sizeof(float) + s.tell();

	if (!m_replicatorPeers->send(peerId, &msg, msgSize, true))
		log::error << L"Unable to send \"I am\" to peer " << peerId << Endl;
}

	}
}
