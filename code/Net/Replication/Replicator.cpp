#include "Core/Io/MemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Timer/Timer.h"
#include "Net/Replication/CompactSerializer.h"
#include "Net/Replication/IReplicatorPeers.h"
#include "Net/Replication/Message.h"
#include "Net/Replication/Replicator.h"
#include "Net/Replication/State/StateTemplate.h"

namespace traktor
{
	namespace net
	{
		namespace
		{

const handle_t c_broadcastHandle = 0UL;
const float c_initialTimeOffset = 0.05f;
const float c_maxOffsetAdjust = 0.5f;
const float c_nearDistance = 15.0f;
const float c_farDistance = 150.0f;
const float c_nearTimeUntilTx = 1.0f / 15.0f;
const float c_farTimeUntilTx = 1.0f / 8.0f;
const float c_timeUntilIAm = 6.0f;
const float c_timeUntilPing = 1.5f;
const float c_peerTimeout = 20.0f;
const float c_maxExtrapolateTime = 4.0f;
const float c_errorStateThreshold = 0.5f;
const uint32_t c_errorLossThreshold = 10000;
const uint32_t c_maxPendingPing = 16;
const uint32_t c_maxErrorCount = 4;
const uint32_t c_maxDeltaStates = 8;
Timer g_timer;

#define T_REPLICATOR_DEBUG(x) traktor::log::info << x << traktor::Endl

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.Replicator", Replicator, Object)

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.Replicator.IListener", Replicator::IListener, Object)

Replicator::Replicator()
:	m_id(0)
,	m_origin(0.0f, 0.0f, 0.0f, 1.0f)
,	m_time(0.0f)
,	m_pingCount(0)
,	m_timeUntilPing(0.0f)
{
	m_id = uint32_t(g_timer.getElapsedTime());
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
		peer.state = PsInitial;
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
	std::vector< handle_t > handles;
	uint8_t iframeData[Message::StateSize];
	uint8_t frameData[Message::StateSize];
	handle_t handle;
	Message msg;

	// Massage replicator peers back-end first and
	// get fresh list of peer handles.
	m_replicatorPeers->update();
	m_replicatorPeers->getPeerHandles(handles);

	// Keep list of unfresh handles.
	for (std::map< handle_t, Peer >::iterator i = m_peers.begin(); i != m_peers.end(); )
	{
		if (std::find(handles.begin(), handles.end(), i->first) != handles.end())
		{
			++i;
			continue;
		}

		Peer& peer = i->second;

		if (peer.state == PsEstablished)
		{
			T_REPLICATOR_DEBUG(L"WARNING: Peer " << i->first << L" connection suddenly terminated");

			// Need to notify listeners immediately as peer becomes dismounted.
			for (RefArray< IListener >::iterator j = m_listeners.begin(); j != m_listeners.end(); ++j)
				(*j)->notify(this, 0, IListener::ReDisconnected, i->first, 0);

			if (peer.ghost)
			{
				peer.ghost->~Ghost(); getAllocator()->free(peer.ghost);
				peer.ghost = 0;
			}
		}

		i = m_peers.erase(i);
	}

	// Iterate all handles, check error state or send "I am" to new peers.
	for (std::vector< handle_t >::const_iterator i = handles.begin(); i != handles.end(); ++i)
	{
		Peer& peer = m_peers[*i];

		if (peer.state == PsDisconnected)
			continue;

		// Issue "I am" to unestablished peers.
		if (peer.state == PsInitial)
		{
			if ((peer.timeUntilTx -= dT) <= 0.0f)
			{
				T_REPLICATOR_DEBUG(L"OK: Unestablished peer found; sending \"I am\" to peer " << *i);
				sendIAm(*i, 0, m_id);
				peer.timeUntilTx = c_timeUntilIAm;
			}
		}

		// Check if peer doesn't respond, timeout;ed or unable to communicate.
		if (peer.state == PsEstablished)
		{
			T_ASSERT (peer.lastTime > 0.0f);

			bool failing = false;

			float T = m_time - peer.lastTime;
			if (T > c_peerTimeout && peer.packetCount > 0)
			{
				T_REPLICATOR_DEBUG(L"WARNING: Peer " << *i << L" timeout, no packet in " << int32_t(T * 1000.0f) << L" ms");
				failing = true;
			}

			if (peer.pendingPing > c_maxPendingPing)
			{
				T_REPLICATOR_DEBUG(L"WARNING: Peer " << *i << L" doesn't respond to ping");
				failing = true;
			}

			if (peer.errorCount > c_maxErrorCount || peer.lossDelta > c_errorLossThreshold)
			{
				T_REPLICATOR_DEBUG(L"WARNING: Peer " << *i << L" failing, unable to communicate with peer");
				failing = true;
			}

			if (failing)
			{
				// Peer should disconnect from the network, so send message (if able) to peer.
				sendDisconnect(*i);

				// Need to notify listeners immediately as peer becomes dismounted.
				for (RefArray< IListener >::iterator j = m_listeners.begin(); j != m_listeners.end(); ++j)
					(*j)->notify(this, 0, IListener::ReDisconnected, *i, 0);

				if (peer.ghost)
				{
					peer.ghost->~Ghost(); getAllocator()->free(peer.ghost);
					peer.ghost = 0;
				}

				peer.state = PsDisconnected;
				peer.iframe = 0;
			}
		}
	}

	// Broadcast my state to all peers.
	if (m_state && m_stateTemplate)
	{
		// Pack iframe from current state.
		uint32_t iframeSize = m_stateTemplate->pack(
			m_state,
			iframeData,
			sizeof(iframeData)
		);

		// Send state to all connected peers.
		msg.time = uint32_t(m_time * 1000.0f);
		for (std::map< handle_t, Peer >::iterator i = m_peers.begin(); i != m_peers.end(); ++i)
		{
			Peer& peer = i->second;

			if (peer.state != PsEstablished || !peer.ghost)
				continue;
			if ((peer.timeUntilTx -= dT) > 0.0f)
				continue;
			if (!m_replicatorPeers->sendReady(i->first))
				continue;

			uint32_t msgSize = Message::HeaderSize + sizeof(uint8_t);

			// Send delta frames only if we've successfully sent
			// an iframe and we're not experiencing package loss.
			if (
				peer.iframe &&
				peer.lossDelta == 0 &&
				peer.stateCount % c_maxDeltaStates > 0
			)
			{
				// Pack delta frame from last sent state.
				uint32_t frameSize = m_stateTemplate->pack(
					peer.iframe,
					m_state,
					frameData,
					sizeof(frameData)
				);

				if (frameSize < iframeSize)
				{
					std::memcpy(msg.state.data, frameData, frameSize);
					msgSize += frameSize;
					msg.type = MtDeltaState;
				}
				else
				{
					std::memcpy(msg.state.data, iframeData, iframeSize);
					msgSize += iframeSize;
					msg.type = MtFullState;
					peer.stateCount = 0;
				}
			}
			else
			{
				std::memcpy(msg.state.data, iframeData, iframeSize);
				msgSize += iframeSize;
				msg.type = MtFullState;
				peer.stateCount = 0;
			}

			msg.state.sequence = peer.txSequence;

			if (m_replicatorPeers->send(i->first, &msg, msgSize, false))
			{
				if (peer.lossDelta == 0)
				{
					if (peer.ghost->stateTemplate)
					{
						float distanceToPeer = (peer.ghost->origin - m_origin).xyz0().length();
						float t = clamp((distanceToPeer - c_nearDistance) / (c_farDistance - c_nearDistance), 0.0f, 1.0f);
						peer.timeUntilTx = lerp(c_nearTimeUntilTx, c_farTimeUntilTx, t);
					}
					else
						peer.timeUntilTx = c_nearTimeUntilTx;
				}
				else
					peer.timeUntilTx = c_farTimeUntilTx;

				peer.errorCount = 0;
				peer.stateCount++;
				peer.txSequence++;
				peer.iframe = m_state;
			}
			else
			{
				log::error << L"ERROR: Unable to send state to peer " << i->first << L" (" << peer.errorCount << L")" << Endl;
				peer.timeUntilTx = c_farTimeUntilTx;
				peer.errorCount++;
				peer.stateCount = 0;
				peer.iframe = 0;
			}
		}
	}

	// Send events to peers.
	if (!m_eventsOut.empty())
	{
		std::list< Event > eventsOut;
		for (std::list< Event >::const_iterator i = m_eventsOut.begin(); i != m_eventsOut.end(); ++i)
		{
			msg.type = MtEvent;
			msg.time = uint32_t(m_time * 1000.0f);

			MemoryStream s(msg.event.data, sizeof(msg.event.data), false, true);
			CompactSerializer cs(&s, &m_eventTypes[0]);
			cs.writeObject(i->object);
			cs.flush();

			uint32_t msgSize = sizeof(uint8_t) + sizeof(uint32_t) + s.tell();

			if (i->handle == c_broadcastHandle)
			{
				for (std::map< handle_t, Peer >::iterator j = m_peers.begin(); j != m_peers.end(); ++j)
				{
					if (j->second.state != PsEstablished)
						continue;

					if (m_replicatorPeers->send(j->first, &msg, msgSize, true))
					{
						j->second.errorCount = 0;
#if defined(T_PROFILE_REPLICATOR)
						m_profileSent[&type_of(i->object)] += msgSize;
#endif
					}
					else
					{
						log::error << L"ERROR: Unable to send event to peer " << j->first << L" (" << j->second.errorCount << L") (1)" << Endl;
						
						// Re-send this event to peer next iteration.
						if (j->second.errorCount == 0)
						{
							Event e = *i;
							e.handle = j->first;
							eventsOut.push_back(e);
						}

						j->second.errorCount++;
					}
				}
			}
			else
			{
				std::map< handle_t, Peer >::iterator j = m_peers.find(i->handle);
				if (j != m_peers.end())
				{
					if (m_replicatorPeers->send(j->first, &msg, msgSize, true))
					{
						j->second.errorCount = 0;
#if defined(T_PROFILE_REPLICATOR)
						m_profileSent[&type_of(i->object)] += msgSize;
#endif
					}
					else
					{
						log::error << L"ERROR: Unable to send event to peer " << j->first << L" (" << j->second.errorCount << L") (2)" << Endl;
						
						// Re-send this event to peer next iteration.
						if (
							j->second.errorCount == 0 &&
							j->second.state == PsEstablished
						)
							eventsOut.push_back(*i);

						j->second.errorCount++;
					}
				}
			}
		}
		m_eventsOut.swap(eventsOut);
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
			if (peer.state == PsEstablished)
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
		std::memset(&msg, 0, sizeof(msg));
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
			T_REPLICATOR_DEBUG(L"OK: Got \"I am\" from peer " << handle << L", sequence " << int32_t(msg.iam.sequence) << L", id " << msg.iam.id);

			// Assume peer time is correct if exceeding my time.
			if (time + c_initialTimeOffset > m_time)
				m_time = time + c_initialTimeOffset;

			if (msg.iam.sequence == 0)
			{
				sendIAm(handle, 1, msg.iam.id);
			}
			else if (msg.iam.sequence == 1 || msg.iam.sequence == 2)
			{
				// "I am" with sequence 1 can only be received if I was the handshake initiator.
				// "I am" with sequence 2 can only be received if I was NOT the handshake initiator.

				if (msg.iam.sequence == 1)
				{
					if (msg.iam.id != m_id)
					{
						T_REPLICATOR_DEBUG(L"ERROR: \"I am\" message with incorrect id from peer " << handle << L"; ignoring");
						continue;
					}
					sendIAm(handle, 2, msg.iam.id);
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

				if (peer.state != PsEstablished)
				{
					peer.state = PsEstablished;
					peer.timeUntilTx = 0.0f;

					// Send ping to peer.
					sendPing(handle);
					++peer.pendingPing;

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

			if (
				peer.state == PsEstablished &&
				peer.ghost
			)
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

			peer.state = PsDisconnected;
			peer.iframe = 0;

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
		else if (msg.type == MtThrottle)	// Received throttle message.
		{
			T_REPLICATOR_DEBUG(L"OK: Received throttle message from peer " << handle);
			m_peers[handle].lossDelta += 4;
		}
		else if (msg.type == MtDisconnect)	// Peer request me to disconnect.
		{
			T_REPLICATOR_DEBUG(L"OK: Received disconnect message from peer " << handle);
			// \fixme
		}
		else if (msg.type == MtFullState || msg.type == MtDeltaState)	// Data message.
		{
			Peer& peer = m_peers[handle];
			if (!peer.ghost)
			{
				T_REPLICATOR_DEBUG(L"ERROR: Peer " << handle << L" partially connected but received MtState; ignoring");
				continue;
			}

			// Check sequence number.
			if (peer.rxSequence != msg.state.sequence)
			{
				T_REPLICATOR_DEBUG(L"WARNING: Packet loss detected; expected " << int32_t(peer.rxSequence) << L", got " << int32_t(peer.txSequence) << L" from peer " << handle);

				// Must receive 10 in order to leave "loss state".
				peer.lossDelta += 10;

				// Do not accept delta state if packet loss detected.
				if (msg.type == MtDeltaState)
				{
					peer.rxSequence = msg.state.sequence + 1;
					continue;
				}

				// Send throttle message to errornous peer.
				sendThrottle(handle);
			}
			else if (peer.lossDelta > 0)
				peer.lossDelta--;

			peer.rxSequence = msg.state.sequence + 1;

			bool stateValid = false;

			// Ignore old messages; as we're using unreliable transportation
			// messages can arrive out-of-order.
			if (time > peer.lastTime + 1e-4f)
			{
				if (time + peer.latencyMinimum > m_time + 1e-5f)
				{
					// Adjust time; adjust only with 75% of the difference in order
					// to stabilize synchronization over multiple iterations.
					float offset = time + peer.latencyMinimum - m_time;
					float adjust = min(offset * 0.75f, c_maxOffsetAdjust);
					m_time += adjust;

					// Also adjust all old states as well.
					for (std::map< handle_t, Peer >::iterator i = m_peers.begin(); i != m_peers.end(); ++i)
					{
						Ghost* ghost = i->second.ghost;
						if (!ghost)
							continue;

						if (ghost->Sn2)
							ghost->Tn2 += adjust;
						if (ghost->Sn1)
							ghost->Tn1 += adjust;
						if (ghost->S0)
							ghost->T0 += adjust;
					}

					// Adjust on all queued events also.
					for (std::list< Event >::iterator i = m_eventsIn.begin(); i != m_eventsIn.end(); ++i)
						i->time += adjust;
					for (std::list< Event >::iterator i = m_eventsOut.begin(); i != m_eventsOut.end(); ++i)
						i->time += adjust;
				}

				if (peer.ghost->stateTemplate)
				{
					Ref< const State > state;
					
					if (msg.type == MtFullState)
						state = peer.ghost->stateTemplate->unpack(msg.state.data, sizeof(msg.state.data));
					else
					{
						if (peer.ghost->S0)
							state = peer.ghost->stateTemplate->unpack(peer.ghost->S0, msg.state.data, sizeof(msg.state.data));
						else
							T_REPLICATOR_DEBUG(L"ERROR: Received delta state from peer " << handle << L" but have no iframe; state ignored");
					}

					if (state)
					{
						if (time - peer.lastTime < c_errorStateThreshold)
						{
							peer.ghost->Sn2 = peer.ghost->Sn1;
							peer.ghost->Tn2 = peer.ghost->Tn1;
							peer.ghost->Sn1 = peer.ghost->S0;
							peer.ghost->Tn1 = peer.ghost->T0;
							peer.ghost->S0 = state;
							peer.ghost->T0 = time;
						}
						else
						{
							T_REPLICATOR_DEBUG(L"WARNING: Ghost state unreasonably old from peer " << handle << L", " << int32_t((time - peer.lastTime) * 1000.0f) << L" ms");
							peer.ghost->Sn2 = 0;
							peer.ghost->Tn2 = 0.0f;
							peer.ghost->Sn1 = 0;
							peer.ghost->Tn1 = 0.0f;
							peer.ghost->S0 = state;
							peer.ghost->T0 = time;
						}
						stateValid = true;
					}
					else
						T_REPLICATOR_DEBUG(L"ERROR: Unable to unpack ghost state (1)");
				}

				peer.packetCount++;
				peer.lastTime = time;
			}
			else
			{
				T_REPLICATOR_DEBUG(L"OK: Received out-of-order package from peer " << handle);

				// Received an old out-of-order package.
				if (peer.ghost->stateTemplate)
				{
					Ref< const State > state;
					
					if (msg.type == MtFullState)
						state = peer.ghost->stateTemplate->unpack(msg.state.data, sizeof(msg.state.data));
					else
					{
						Ref< const State > Sn;

						if (time > peer.ghost->Tn1)
							Sn = peer.ghost->Sn1;
						else if (time > peer.ghost->Tn2)
							Sn = peer.ghost->Sn2;

						if (Sn)
							state = peer.ghost->stateTemplate->unpack(Sn, msg.state.data, sizeof(msg.state.data));
						else
							T_REPLICATOR_DEBUG(L"ERROR: Received delta state from peer " << handle << L" but have no iframe; state ignored");
					}

					if (state)
					{
						if (time > peer.ghost->Tn1)
						{
							peer.ghost->Sn2 = peer.ghost->Sn1;
							peer.ghost->Tn2 = peer.ghost->Tn1;
							peer.ghost->Sn1 = state;
							peer.ghost->Tn1 = time;
							stateValid = true;
						}
						else if (time > peer.ghost->Tn2)
						{
							peer.ghost->Sn2 = state;
							peer.ghost->Tn2 = time;
							stateValid = true;
						}
					}
					else
						T_REPLICATOR_DEBUG(L"ERROR: Unable to unpack ghost state (2)");
				}

				peer.packetCount++;
			}

			// Put an input event to notify listeners about new state.
			if (stateValid)
			{
				Event evt;
				evt.eventId = IListener::ReState;
				evt.handle = handle;
				evt.object = 0;
				m_eventsIn.push_back(evt);
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
			{
				if (time + peer.latencyMinimum > m_time + 1e-5f)
				{
					// Adjust time; adjust only with 75% of the difference in order
					// to stabilize synchronization over multiple iterations.
					float offset = time + peer.latencyMinimum - m_time;
					float adjust = min(offset * 0.75f, c_maxOffsetAdjust);
					m_time += adjust;

					// Also adjust all old states as well.
					for (std::map< handle_t, Peer >::iterator i = m_peers.begin(); i != m_peers.end(); ++i)
					{
						Ghost* ghost = i->second.ghost;
						if (!ghost)
							continue;

						if (ghost->Sn2)
							ghost->Tn2 += adjust;
						if (ghost->Sn1)
							ghost->Tn1 += adjust;
						if (ghost->S0)
							ghost->T0 += adjust;
					}

					// Adjust on all queued events also.
					for (std::list< Event >::iterator i = m_eventsIn.begin(); i != m_eventsIn.end(); ++i)
						i->time += adjust;
					for (std::list< Event >::iterator i = m_eventsOut.begin(); i != m_eventsOut.end(); ++i)
						i->time += adjust;
				}

				peer.packetCount++;
				peer.lastTime = time;
			}

			MemoryStream s(msg.event.data, sizeof(msg.event.data), true, false);
			Ref< ISerializable > eventObject = CompactSerializer(&s, &m_eventTypes[0]).readObject< ISerializable >();

			// Put an input event to notify listeners about received event.
			Event e;
			e.time = time;
			e.eventId = IListener::ReBroadcastEvent;
			e.handle = handle;
			e.object = eventObject;
			m_eventsIn.push_back(e);

#if defined(T_PROFILE_REPLICATOR)
			m_profileReceived[&type_of(eventObject)] += s.tell();
#endif
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

#if defined(T_PROFILE_REPLICATOR)
	// Dump event information.
	if (int32_t(m_time / 10.0f) != int32_t((m_time + dT) / 10.0f))
	{
		log::info << L"Sent events" << Endl;
		for (std::map< const TypeInfo*, uint32_t >::const_iterator i = m_profileSent.begin(); i != m_profileSent.end(); ++i)
			log::info << L"\t" << i->first->getName() << L" " << i->second << L" byte(s)" << Endl;

		log::info << L"Received events" << Endl;
		for (std::map< const TypeInfo*, uint32_t >::const_iterator i = m_profileReceived.begin(); i != m_profileReceived.end(); ++i)
			log::info << L"\t" << i->first->getName() << L" " << i->second << L" byte(s)" << Endl;
	}
#endif

	m_time += dT;
}

void Replicator::setOrigin(const Vector4& origin)
{
	m_origin = origin;
}

void Replicator::setStateTemplate(const StateTemplate* stateTemplate)
{
	m_stateTemplate = stateTemplate;
	m_state = 0;
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

int32_t Replicator::getBestReversedLatency() const
{
	int32_t latencyBest = 0;
	for (std::map< handle_t, Peer >::const_iterator i = m_peers.begin(); i != m_peers.end(); ++i)
	{
		if (latencyBest > 0)
			latencyBest = std::min(latencyBest, int32_t(i->second.latencyReversed * 1000.0f));
		else
			latencyBest = int32_t(i->second.latencyReversed * 1000.0f);
	}
	return latencyBest;
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
	else if (i->second.state != PsEstablished || !i->second.ghost)
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
	{
		i->second.ghost->stateTemplate = stateTemplate;
		i->second.ghost->Sn2 = 0;
		i->second.ghost->Sn1 = 0;
		i->second.ghost->S0 = 0;
		i->second.ghost->Tn2 = 0.0f;
		i->second.ghost->Tn1 = 0.0f;
		i->second.ghost->T0 = 0.0f;
	}
	else
		T_REPLICATOR_DEBUG(L"ERROR: Trying to get ghost state of unknown peer handle " << peerHandle);
}

Ref< const State > Replicator::getGhostState(handle_t peerHandle, const State* currentState) const
{
	std::map< handle_t, Peer >::const_iterator i = m_peers.find(peerHandle);
	if (i != m_peers.end() && i->second.ghost)
	{
		if (m_time > i->second.ghost->T0 + c_maxExtrapolateTime)
			return 0;

		const StateTemplate* stateTemplate = i->second.ghost->stateTemplate;
		if (stateTemplate)
			return stateTemplate->extrapolate(
				i->second.ghost->Sn2,
				i->second.ghost->Tn2,
				i->second.ghost->Sn1,
				i->second.ghost->Tn1,
				i->second.ghost->S0,
				i->second.ghost->T0,
				currentState,
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

Ref< const State > Replicator::getLoopBackState() const
{
	if (!m_stateTemplate)
		return 0;

	uint8_t data[1200];
	uint32_t size = m_stateTemplate->pack(m_state, data, sizeof(data));
	T_DEBUG(L"Loopback state size : " << size << L" byte(s)");

	return m_stateTemplate->unpack(data, size);
}

void Replicator::sendIAm(handle_t peerHandle, uint8_t sequence, uint32_t id)
{
	Message msg;

	msg.type = MtIAm;
	msg.time = uint32_t(m_time * 1000.0f);
	msg.iam.sequence = sequence;
	msg.iam.id = id;

	uint32_t msgSize = sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint32_t);
	m_replicatorPeers->send(peerHandle, &msg, msgSize, false);
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

void Replicator::sendThrottle(handle_t peerHandle)
{
	Message msg;

	msg.type = MtThrottle;
	msg.time = uint32_t(m_time * 1000.0f);

	uint32_t msgSize = sizeof(uint8_t) + sizeof(uint32_t);
	m_replicatorPeers->send(peerHandle, &msg, msgSize, false);
}

void Replicator::sendDisconnect(handle_t peerHandle)
{
	Message msg;

	msg.type = MtDisconnect;
	msg.time = uint32_t(m_time * 1000.0f);

	uint32_t msgSize = sizeof(uint8_t) + sizeof(uint32_t);
	m_replicatorPeers->send(peerHandle, &msg, msgSize, true);
}

	}
}
