#include <cstring>
#include "Core/Io/MemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Math/Random.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/CompactSerializer.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Timer/Timer.h"
#include "Net/Replication/IReplicatorPeers.h"
#include "Net/Replication/Message.h"
#include "Net/Replication/Replicator.h"
#include "Net/Replication/State/State.h"
#include "Net/Replication/State/StateTemplate.h"

namespace traktor
{
	namespace net
	{
		namespace
		{

const handle_t c_broadcastHandle = 0UL;
const int32_t c_initialTimeOffset = 50;

Timer g_timer;
Random g_random;

#define T_REPLICATOR_DEBUG(x) traktor::log::info << x << traktor::Endl

struct PredHandle
{
	handle_t handle;

	PredHandle(handle_t handle_)
	:	handle(handle_)
	{
	}

	bool operator () (const IReplicatorPeers::PeerInfo info) const
	{
		return info.handle == handle;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.Replicator", Replicator, Object)

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.Replicator.IListener", Replicator::IListener, Object)

Replicator::Replicator()
:	m_id(0)
,	m_origin(Transform::identity())
,	m_time0(0)
,	m_time(0)
,	m_pingCount(0)
,	m_timeUntilPing(0)
{
	m_id = uint32_t(g_timer.getElapsedTime());
}

Replicator::~Replicator()
{
	destroy();
}

bool Replicator::create(IReplicatorPeers* replicatorPeers, const Configuration& configuration)
{
	std::vector< IReplicatorPeers::PeerInfo > info;

	m_configuration = configuration;
	m_replicatorPeers = replicatorPeers;
	m_state = 0;

	m_replicatorPeers->update();
	m_replicatorPeers->getPeers(info);

	// Discard any pending data.
	for (;;)
	{
		Message discard;
		handle_t fromHandle;

		if (receive(&discard, fromHandle) <= 0)
			break;
	}

	// Create non-established entries for each peer.
	for (std::vector< IReplicatorPeers::PeerInfo >::const_iterator i = info.begin(); i != info.end(); ++i)
	{
		Peer& peer = m_peers[i->handle];
		peer.state = PsInitial;
		peer.name = i->name;
		peer.endSite = i->endSite;
	}

	return true;
}

void Replicator::destroy()
{
	if (!m_peers.empty())
	{
		// Send Bye message to all peers.
		for (std::map< handle_t, Peer >::iterator i = m_peers.begin(); i != m_peers.end(); ++i)
		{
			if (i->second.state == PsEstablished)
				sendBye(i->first);
		}

		// Delete all peer control.
		for (std::map< handle_t, Peer >::iterator i = m_peers.begin(); i != m_peers.end(); ++i)
		{
			if (i->second.ghost)
			{
				i->second.ghost->~Ghost();
				getAllocator()->free(i->second.ghost);
				i->second.ghost = 0;
			}
		}
	}

	m_peers.clear();
	m_eventsIn.clear();
	m_eventsOut.clear();
	m_listeners.clear();
	m_eventTypes.clear();

	safeDestroy(m_replicatorPeers);
	m_state = 0;
}

void Replicator::setConfiguration(const Configuration& configuration)
{
	m_configuration = configuration;
}

const Replicator::Configuration& Replicator::getConfiguration() const
{
	return m_configuration;
}

void Replicator::addEventType(const TypeInfo& eventType)
{
	m_eventTypes.push_back(&eventType);
}

void Replicator::addListener(IListener* listener)
{
	m_listeners.push_back(listener);
}

bool Replicator::update(float /*T*/, float dT)
{
	if (!m_replicatorPeers)
		return false;

	int32_t idT = int32_t(dT * 1000.0f);

	updatePeers(idT);

	if (!m_replicatorPeers)
		return false;

	sendState(idT);
	sendEvents();
	sendPings(idT);
	receiveMessages();
	updateTimeSynchronization();
	dispatchEventListeners();

	m_time0 += idT;
	m_time += idT;

	return bool(m_replicatorPeers != 0);
}

handle_t Replicator::getHandle() const
{
	return m_replicatorPeers->getHandle();
}

std::wstring Replicator::getName() const
{
	return m_replicatorPeers->getName();
}

void Replicator::setStatus(uint8_t status)
{
	T_ASSERT (m_replicatorPeers);
	m_replicatorPeers->setStatus(status);
}

void Replicator::setOrigin(const Transform& origin)
{
	m_origin = origin;
}

void Replicator::setStateTemplate(const StateTemplate* stateTemplate)
{
	// Replace template.
	m_stateTemplate = stateTemplate;

	// Nuke states from previous template.
	m_state = 0;
	for (std::map< handle_t, Peer >::iterator i = m_peers.begin(); i != m_peers.end(); ++i)
		i->second.iframe = 0;
}

void Replicator::setState(const State* state)
{
	T_FATAL_ASSERT (m_stateTemplate);
	
	// Clear count-down timer if state is critical which
	// will cause the state to be replicated at next update
	// to all peers.
	if (state != 0 && m_stateTemplate->critical(m_state, state))
	{
		for (std::map< handle_t, Peer >::iterator i = m_peers.begin(); i != m_peers.end(); ++i)
		{
			Peer& peer = i->second;
			if (peer.state == PsEstablished)
				peer.timeUntilTx = 0.0f;
		}
	}

	m_state = state;
}

void Replicator::sendEvent(handle_t peerHandle, const ISerializable* eventObject)
{
	EventOut e;
	e.eventId = 0;
	e.handle = peerHandle;
	e.object = eventObject;
	m_eventsOut.push_back(e);
}

void Replicator::broadcastEvent(const ISerializable* eventObject)
{
	EventOut e;
	e.eventId = 0;
	e.handle = c_broadcastHandle;
	e.object = eventObject;
	m_eventsOut.push_back(e);
}

bool Replicator::isPrimary() const
{
	return m_replicatorPeers->getPrimaryPeerHandle() == m_replicatorPeers->getHandle();
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
	std::map< handle_t, Peer >::const_iterator i = m_peers.find(peerHandle);
	return i != m_peers.end() ? i->second.name : L"";
}

Object* Replicator::getPeerEndSite(handle_t peerHandle) const
{
	std::map< handle_t, Peer >::const_iterator i = m_peers.find(peerHandle);
	return i != m_peers.end() ? i->second.endSite : 0;
}

uint8_t Replicator::getPeerStatus(handle_t peerHandle) const
{
	std::map< handle_t, Peer >::const_iterator i = m_peers.find(peerHandle);
	return i != m_peers.end() ? i->second.status : 0;
}

int32_t Replicator::getPeerLatency(handle_t peerHandle) const
{
	std::map< handle_t, Peer >::const_iterator i = m_peers.find(peerHandle);
	return i != m_peers.end() ? i->second.latencyMinimum : 0;
}

int32_t Replicator::getPeerReversedLatency(handle_t peerHandle) const
{
	std::map< handle_t, Peer >::const_iterator i = m_peers.find(peerHandle);
	return i != m_peers.end() ? i->second.latencyReversed : 0;
}

int32_t Replicator::getBestReversedLatency() const
{
	int32_t latencyBest = 0;
	for (std::map< handle_t, Peer >::const_iterator i = m_peers.begin(); i != m_peers.end(); ++i)
	{
		if (latencyBest > 0)
			latencyBest = std::min(latencyBest, i->second.latencyReversed);
		else
			latencyBest = i->second.latencyReversed;
	}
	return latencyBest;
}

int32_t Replicator::getWorstReversedLatency() const
{
	int32_t latencyWorst = 0;
	for (std::map< handle_t, Peer >::const_iterator i = m_peers.begin(); i != m_peers.end(); ++i)
		latencyWorst = std::max(latencyWorst, i->second.latencyReversed);
	return latencyWorst;
}

bool Replicator::isPeerConnected(handle_t peerHandle) const
{
	std::map< handle_t, Peer >::const_iterator i = m_peers.find(peerHandle);
	if (i == m_peers.end() || i->second.state != PsEstablished)
		return false;
	else
		return true;
}

bool Replicator::isPeerRelayed(handle_t peerHandle) const
{
	std::map< handle_t, Peer >::const_iterator i = m_peers.find(peerHandle);
	return i != m_peers.end() ? !i->second.direct : false;
}

bool Replicator::setPeerPrimary(handle_t peerHandle)
{
	T_ASSERT (m_replicatorPeers);
	return m_replicatorPeers->setPrimaryPeerHandle(peerHandle);
}

handle_t Replicator::getPrimaryPeerHandle() const
{
	for (std::map< handle_t, Peer >::const_iterator i = m_peers.begin(); i != m_peers.end(); ++i)
	{
		if (isPeerPrimary(i->first))
			return i->first;
	}
	return 0;
}

bool Replicator::isPeerPrimary(handle_t peerHandle) const
{
	return m_replicatorPeers->getPrimaryPeerHandle() == peerHandle;
}

bool Replicator::areAllPeersConnected() const
{
	for (std::map< handle_t, Peer >::const_iterator i = m_peers.begin(); i != m_peers.end(); ++i)
	{
		if (i->second.state != PsEstablished)
			return false;
	}
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

void Replicator::setGhostOrigin(handle_t peerHandle, const Transform& origin)
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
		i->second.ghost->Tn2 = 0;
		i->second.ghost->Tn1 = 0;
		i->second.ghost->T0 = 0;
	}
	else
		T_REPLICATOR_DEBUG(L"ERROR: Trying to get ghost state of unknown peer handle " << peerHandle);
}

const StateTemplate* Replicator::getGhostStateTemplate(handle_t peerHandle) const
{
	std::map< handle_t, Peer >::const_iterator i = m_peers.find(peerHandle);
	if (i != m_peers.end() && i->second.ghost)
		return i->second.ghost->stateTemplate;
	else
		return 0;
}

float Replicator::getGhostStateTime(handle_t peerHandle) const
{
	std::map< handle_t, Peer >::const_iterator i = m_peers.find(peerHandle);
	if (i != m_peers.end() && i->second.ghost)
		return i->second.ghost->T0 / 1000.0f;
	else
	{
		T_REPLICATOR_DEBUG(L"ERROR: Trying to get ghost state of unknown peer handle " << peerHandle);
		return 0.0f;
	}
}

Ref< const State > Replicator::getGhostState(handle_t peerHandle, float timeOffset) const
{
	std::map< handle_t, Peer >::const_iterator i = m_peers.find(peerHandle);
	if (i != m_peers.end() && i->second.ghost)
	{
		const Peer& peer = i->second;

		float delta = timeOffset - peer.ghost->T0 / 1000.0f;
		if (abs(delta) > m_configuration.maxExtrapolationDelta)
			T_REPLICATOR_DEBUG(L"WARNING: Peer " << peer.name << L" extrapolation delta out-of-range, delta = " << delta);

		delta = clamp(delta, -m_configuration.maxExtrapolationDelta, m_configuration.maxExtrapolationDelta);
		timeOffset = peer.ghost->T0 / 1000.0f + delta;

		const StateTemplate* stateTemplate = peer.ghost->stateTemplate;
		if (stateTemplate)
			return stateTemplate->extrapolate(
				peer.ghost->Sn2,
				peer.ghost->Tn2 / 1000.0f,
				peer.ghost->Sn1,
				peer.ghost->Tn1 / 1000.0f,
				peer.ghost->S0,
				peer.ghost->T0 / 1000.0f,
				timeOffset
			);
		else
			return 0;
	}
	else
	{
		T_REPLICATOR_DEBUG(L"ERROR: Trying to get ghost state of unknown peer handle " << peerHandle);
		return 0;
	}
}

Ref< const State > Replicator::getLoopBackState() const
{
	if (m_stateTemplate)
	{
		uint8_t data[Message::MessageSize];
		uint32_t size = m_stateTemplate->pack(m_state, data, sizeof(data));
		return m_stateTemplate->unpack(data, size);
	}
	else
		return 0;
}

void Replicator::updatePeers(int32_t dT)
{
	std::vector< IReplicatorPeers::PeerInfo > info;
	info.reserve(m_peers.size());

	// Massage replicator peers back-end first and
	// then get fresh list of peer handles.
	if (!m_replicatorPeers->update())
	{
		T_REPLICATOR_DEBUG(L"ERROR: Connection to game lost (1)");
		destroy();
		return;
	}

	m_replicatorPeers->getPeers(info);

	// Keep list of un-fresh handles.
	for (std::map< handle_t, Peer >::iterator i = m_peers.begin(); i != m_peers.end(); )
	{
		if (std::find_if(info.begin(), info.end(), PredHandle(i->first)) != info.end())
		{
			++i;
			continue;
		}

		Peer& peer = i->second;
		if (peer.state == PsEstablished)
		{
			T_REPLICATOR_DEBUG(L"WARNING: Peer " << i->second.name << L" connection suddenly terminated");

			// Need to notify listeners immediately as peer becomes dismounted.
			for (RefArray< IListener >::iterator j = m_listeners.begin(); j != m_listeners.end(); ++j)
				(*j)->notify(this, 0, IListener::ReLost, i->first, 0);

			if (peer.ghost)
			{
				peer.ghost->~Ghost(); getAllocator()->free(peer.ghost);
				peer.ghost = 0;
			}

			peer.state = PsDisconnected;
			peer.iframe = 0;
			peer.pendingPing = 0;
			peer.errorCount = 0;

			++i;
		}
		else
			m_peers.erase(i++);
	}

	// Iterate all handles, check error state or send "I am" to new peers.
	for (std::vector< IReplicatorPeers::PeerInfo >::const_iterator i = info.begin(); i != info.end(); ++i)
	{
		Peer& peer = m_peers[i->handle];

		// Issue "I am" to unestablished peers.
		if (peer.state == PsInitial)
		{
			peer.name = i->name;
			peer.endSite = i->endSite;

			if ((peer.timeUntilTx -= dT) <= 0)
			{
				T_REPLICATOR_DEBUG(L"OK: Unestablished peer found; sending \"I am\" to peer \"" << peer.name << L"\" (" << peer.pendingIAm << L")");

				if (sendIAm(i->handle, 0, m_id))
					peer.pendingIAm++;
				else
				{
					T_REPLICATOR_DEBUG(L"ERROR: Unable to send \"I am\" to peer \"" << peer.name << L"\"");
					peer.pendingIAm = 0;
				}

				peer.timeUntilTx = int32_t(m_configuration.timeUntilIAm * (1.0f + g_random.nextFloat()));
			}
		}

		// Check if peer doesn't respond, timeout;ed or unable to communicate.
		else if (peer.state == PsEstablished)
		{
			if (peer.errorCount >= m_configuration.maxErrorCount)
			{
				T_REPLICATOR_DEBUG(L"WARNING: Peer \"" << peer.name << L"\" failing, unable to communicate with peer");

				// Need to notify listeners immediately as peer becomes dismounted.
				for (RefArray< IListener >::iterator j = m_listeners.begin(); j != m_listeners.end(); ++j)
					(*j)->notify(this, 0, IListener::ReLost, i->handle, 0);

				if (peer.ghost)
				{
					peer.ghost->~Ghost(); getAllocator()->free(peer.ghost);
					peer.ghost = 0;
				}

				peer.state = PsDisconnected;
				peer.iframe = 0;
				peer.pendingPing = 0;
				peer.errorCount = 0;
			}
		}

		// Save info about peer.
		peer.status = i->status;
		peer.direct = i->direct;
	}
}

void Replicator::sendState(int32_t dT)
{
	uint8_t iframeData[Message::DataSize];
	uint8_t frameData[Message::DataSize];
	Message msg;

	// Broadcast my state to all peers.
	if (!m_state || !m_stateTemplate)
		return;

	// Pack iframe from current state.
	uint32_t iframeSize = m_stateTemplate->pack(
		m_state,
		iframeData,
		sizeof(iframeData)
	);

	// Send state to all connected peers.
	msg.time = m_time;
	for (std::map< handle_t, Peer >::iterator i = m_peers.begin(); i != m_peers.end(); ++i)
	{
		Peer& peer = i->second;

		if (peer.state != PsEstablished || !peer.ghost || !peer.ghost->stateTemplate)
			continue;

		bool shouldSend = bool((peer.timeUntilTx -= dT) <= 0);
		if (!shouldSend)
			continue;

		uint32_t msgSize = Message::HeaderSize;

		// Send delta frames only if we've successfully sent
		// an iframe and we're not experiencing package loss.
		if (
			m_configuration.deltaCompression &&
			peer.iframe &&
			peer.stateCount % m_configuration.maxDeltaStates > 0
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
			peer.iframe = 0;
		}

		if (send(i->first, &msg, msgSize, false))
		{
			peer.timeUntilTx = m_configuration.farTimeUntilTx;
			peer.errorCount = 0;
			peer.stateCount++;
			peer.iframe = m_state;
		}
		else
		{
			if (!peer.errorCount)
				log::error << L"ERROR: Unable to send state to peer " << peer.name << Endl;
			peer.timeUntilTx = m_configuration.farTimeUntilTx;
			peer.errorCount++;
			peer.stateCount = 0;
			peer.iframe = 0;
		}

		Vector4 ghostToPlayer = m_origin.translation() - peer.ghost->origin.translation();
		Scalar distanceToPeer = ghostToPlayer.length();
		float t = clamp((distanceToPeer - m_configuration.nearDistance) / (m_configuration.farDistance - m_configuration.nearDistance), 0.0f, 1.0f);
		peer.timeUntilTx = (int32_t)lerp(m_configuration.nearTimeUntilTx, m_configuration.farTimeUntilTx + int32_t((g_random.nextFloat() - 0.5f) * (m_configuration.farTimeUntilTx - m_configuration.nearTimeUntilTx) * 0.5f), t);
	}
}

void Replicator::sendEvents()
{
	std::list< EventOut > eventsOut;
	Message msg;

	if (m_eventsOut.empty())
		return;

	for (std::map< handle_t, Peer >::iterator i = m_peers.begin(); i != m_peers.end(); ++i)
	{
		Peer& peer = i->second;

		if (peer.state != PsEstablished || !peer.ghost)
			continue;

		std::vector< EventOut > peerEventsOut;

		// Collect events to peer.
		for (std::list< EventOut >::const_iterator j = m_eventsOut.begin(); j != m_eventsOut.end(); ++j)
		{
			if (j->handle == c_broadcastHandle || j->handle == i->first)
			{
				peerEventsOut.push_back(*j);
				peerEventsOut.back().handle = i->first;
			}
		}

		if (peerEventsOut.empty())
			continue;

		// Pack as many events into same message as possible.
		for (uint32_t j = 0; j < peerEventsOut.size(); )
		{
			uint32_t count = uint32_t(peerEventsOut.size() - j);
			T_ASSERT (count > 0);

			msg.type = MtEvent0;
			msg.time = m_time;

			uint8_t data[sizeof(msg.event.data) + 1];	// Add a padding to be able to detect if a single object exceed max size.
			uint8_t* msgDataPtr = msg.event.data;
			uint8_t* msgDataEndPtr = msgDataPtr + sizeof(msg.event.data);

			uint32_t jj = j;
			while (jj < peerEventsOut.size() && msg.type < MtEvent4)
			{
				MemoryStream s(data, sizeof(data), false, true);
				CompactSerializer cs(&s, &m_eventTypes[0], m_eventTypes.size());
				cs.writeObject(peerEventsOut[jj].object);
				cs.flush();

				uint32_t dataSize = s.tell();
				if (dataSize > uint32_t(msgDataEndPtr - msgDataPtr))
					break;

				std::memcpy(msgDataPtr, data, dataSize);
				msgDataPtr += dataSize;

				++jj;
				++msg.type;
			}

			uint32_t msgSize = Message::HeaderSize + uint32_t(msgDataPtr - msg.event.data);

			if (send(i->first, &msg, msgSize, true))
			{
				i->second.errorCount = 0;
				j = jj;
			}
			else
			{
				if (!i->second.errorCount)
					log::error << L"ERROR: Unable to send event(s) to peer " << peer.name << Endl;
				
				for (; j < jj; ++j)
					eventsOut.push_back(peerEventsOut[j]);

				i->second.errorCount++;
				break;
			}
		}
	}

	m_eventsOut.swap(eventsOut);
}

void Replicator::sendPings(int32_t dT)
{
	m_timeUntilPing -= dT;
	if (m_timeUntilPing > 0)
		return;

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

		m_timeUntilPing = m_configuration.timeUntilPing / m_peers.size();
	}
	else
		m_timeUntilPing = m_configuration.timeUntilPing;
}

void Replicator::receiveMessages()
{
	handle_t handle;
	Message msg;
	int32_t size;

	// Read messages from any peer.
	while (m_replicatorPeers)
	{
		std::memset(&msg, 0, sizeof(msg));
		if ((size = receive(&msg, handle)) <= 0)
			break;

		// Always handle handshake messages.
		if (msg.type == MtIAm)
		{
			// Assume peer time is correct if exceeding my time.
			int32_t offset = msg.time + c_initialTimeOffset - m_time;
			if (offset > 0)
				adjustTime(offset);

			if (msg.iam.sequence == 0)
			{
				sendIAm(handle, 1, msg.iam.id);
			}
			else if (msg.iam.sequence == 1 || msg.iam.sequence == 2)
			{
				Peer& peer = m_peers[handle];

				// "I am" with sequence 1 can only be received if I was the handshake initiator.
				// "I am" with sequence 2 can only be received if I was NOT the handshake initiator.

				if (msg.iam.sequence == 1)
				{
					if (msg.iam.id != m_id)
					{
						T_REPLICATOR_DEBUG(L"ERROR: \"I am\" message with incorrect id from peer \"" << peer.name << L"\"; ignoring");
						continue;
					}
					sendIAm(handle, 2, msg.iam.id);
				}

				if (!peer.ghost)
				{
					// Create ghost data.
					void* ghostMem = getAllocator()->alloc(sizeof(Ghost), 16, "Ghost");

					peer.ghost = new (ghostMem) Ghost();
					peer.ghost->origin = m_origin;
					peer.ghost->Tn2 = 0;
					peer.ghost->Tn1 = 0;
					peer.ghost->T0 = 0;
				}

				if (peer.state != PsEstablished)
				{
					peer.state = PsEstablished;
					peer.timeUntilTx = 0;

					// Send ping to peer.
					sendPing(handle);
					++peer.pendingPing;

					// Issue connect event to listeners.
					EventIn evt;
					evt.eventId = IListener::ReConnected;
					evt.handle = handle;
					evt.object = 0;
					m_eventsIn.push_back(evt);

					T_REPLICATOR_DEBUG(L"OK: Peer \"" << peer.name << L"\" connection established");
				}

				peer.lastTimeRemote = std::max< int32_t >(msg.time, peer.lastTimeRemote + 1);
				peer.lastTimeLocal = m_time;

				--peer.pendingIAm;
			}
		}
		else if (msg.type == MtBye)
		{
			Peer& peer = m_peers[handle];
			if (
				peer.state == PsEstablished &&
				peer.ghost
			)
			{
				T_REPLICATOR_DEBUG(L"OK: Established peer \"" << peer.name << L"\" gracefully disconnected; issue listener event");

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

			peer.lastTimeRemote = std::max< int32_t >(msg.time, peer.lastTimeRemote + 1);
			peer.lastTimeLocal = m_time;
		}
		else if (msg.type == MtPing)
		{
			// I've got pinged; reply with a pong.
			sendPong(handle, msg.time);
		}
		else if (msg.type == MtPong)
		{
			// I've received a pong reply from an earlier ping;
			// calculate round-trip time.
			Peer& peer = m_peers[handle];

			int32_t pingTime = msg.pong.time0;
			int32_t roundTrip = max(m_time0 - pingTime, 0);

			peer.roundTrips.push_back(roundTrip);

			// Get lowest and median round-trips and calculate latencies.
			int32_t minrt = roundTrip;
			int32_t sorted[MaxRoundTrips];
			for (uint32_t i = 0; i < peer.roundTrips.size(); ++i)
			{
				sorted[i] = peer.roundTrips[i];
				minrt = min(minrt, peer.roundTrips[i]);
			}

			std::sort(&sorted[0], &sorted[peer.roundTrips.size()]);

			peer.latencyMedian = sorted[peer.roundTrips.size() / 2] / 2;
			peer.latencyMinimum = minrt / 2;
			peer.latencyReversed = msg.pong.latency;

			if (peer.pendingPing > 0)
				--peer.pendingPing;
		}
		else if (msg.type == MtFullState || msg.type == MtDeltaState)	// Data message.
		{
			Peer& peer = m_peers[handle];
			if (!peer.ghost)
				continue;

			int32_t stateDataSize = size - Message::HeaderSize;
			if (stateDataSize <= 0)
				continue;

			Ref< const State > state;

			// Ignore old messages; as we're using unreliable transportation
			// messages can arrive out-of-order.
			if (msg.time > peer.lastTimeRemote)
			{
				// Accumulate time offsets.
				int32_t offset = msg.time + peer.latencyMedian - m_time;
				peer.timeOffsets.push_back(offset < 30 ? offset / 4 : offset / 2);

				if (peer.ghost->stateTemplate)
				{
					if (msg.type == MtFullState)
						state = peer.ghost->stateTemplate->unpack(msg.state.data, stateDataSize);
					else if (peer.ghost->S0)
						state = peer.ghost->stateTemplate->unpack(peer.ghost->S0, msg.state.data, stateDataSize);

					if (state)
					{
						peer.ghost->Sn2 = peer.ghost->Sn1;
						peer.ghost->Tn2 = peer.ghost->Tn1;
						peer.ghost->Sn1 = peer.ghost->S0;
						peer.ghost->Tn1 = peer.ghost->T0;
						peer.ghost->S0 = state;
						peer.ghost->T0 = msg.time;
					}
					else
						T_REPLICATOR_DEBUG(L"ERROR: Unable to unpack state of peer " << peer.name);
				}

				peer.lastTimeLocal = m_time;
				peer.lastTimeRemote = msg.time;

				// Put an input event to notify listeners about new state.
				if (
					peer.ghost &&
					peer.ghost->S0 &&
					peer.ghost->Sn1 &&
					peer.ghost->Sn2
				)
				{
					std::list< EventIn >::iterator it = m_eventsIn.begin();
					for (; it != m_eventsIn.end(); ++it)
					{
						if (it->eventId == IListener::ReState && it->handle == handle)
						{
							it->time = peer.ghost->T0;
							it->object = peer.ghost->S0;
							break;
						}
					}
					if (it == m_eventsIn.end())
					{
						EventIn evt;
						evt.time = peer.ghost->T0;
						evt.eventId = IListener::ReState;
						evt.handle = handle;
						evt.object = peer.ghost->S0;
						m_eventsIn.push_back(evt);
					}
				}
			}
			else
			{
				// Received an old out-of-order package.
				if (peer.ghost->stateTemplate)
				{
					if (msg.type == MtFullState)
						state = peer.ghost->stateTemplate->unpack(msg.state.data, stateDataSize);
					else
					{
						Ref< const State > Sn;

						if (msg.time > peer.ghost->Tn1)
							Sn = peer.ghost->Sn1;
						else if (msg.time > peer.ghost->Tn2)
							Sn = peer.ghost->Sn2;

						if (Sn)
							state = peer.ghost->stateTemplate->unpack(Sn, msg.state.data, stateDataSize);
						else
							T_REPLICATOR_DEBUG(L"ERROR: Received delta state from peer \"" << peer.name << L"\" but have no iframe; state ignored (2)");
					}

					if (state)
					{
						if (msg.time > peer.ghost->Tn1)
						{
							peer.ghost->Sn2 = peer.ghost->Sn1;
							peer.ghost->Tn2 = peer.ghost->Tn1;
							peer.ghost->Sn1 = state;
							peer.ghost->Tn1 = msg.time;
						}
						else if (msg.time > peer.ghost->Tn2)
						{
							peer.ghost->Sn2 = state;
							peer.ghost->Tn2 = msg.time;
						}
					}
					else
						T_REPLICATOR_DEBUG(L"ERROR: Unable to unpack ghost state (2)");
				}
			}
		}
		else if (msg.type >= MtEvent1 && msg.type <= MtEvent4)	// Event message(s).
		{
			Peer& peer = m_peers[handle];
			if (!peer.ghost)
			{
				T_REPLICATOR_DEBUG(L"ERROR: Peer \"" << peer.name << L"\" partially connected but received MtEvent; ignoring");
				continue;
			}

			peer.lastTimeLocal = m_time;
			peer.lastTimeRemote = msg.time;

			if (!m_eventTypes.empty())
			{
				const uint8_t* msgDataPtr = msg.event.data;
				const uint8_t* msgDataEndPtr = msgDataPtr + sizeof(msg.event.data);

				uint32_t eventObjectCount = uint32_t(msg.type - MtEvent0);
				for (uint32_t i = 0; i < eventObjectCount; ++i)
				{
					MemoryStream s(msgDataPtr, uint32_t(msgDataEndPtr - msgDataPtr));
					Ref< ISerializable > eventObject = CompactSerializer(&s, &m_eventTypes[0], m_eventTypes.size()).readObject< ISerializable >();
					if (eventObject)
					{
						EventIn e;
						e.time = msg.time;
						e.eventId = IListener::ReBroadcastEvent;
						e.handle = handle;
						e.object = eventObject;
						m_eventsIn.push_back(e);
					}
					msgDataPtr += s.tell();
				}
			}
		}
	}
}

void Replicator::updateTimeSynchronization()
{
	// Check if all peers have a complete set of time offsets.
	for (std::map< handle_t, Peer >::iterator i = m_peers.begin(); i != m_peers.end(); ++i)
	{
		Peer& peer = i->second;
		if (peer.state == PsEstablished && peer.errorCount == 0 && peer.timeOffsets.size() < peer.timeOffsets.capacity())
			return;
	}

	// Get max median time offset.
	int32_t timeOffset = 0;
	for (std::map< handle_t, Peer >::iterator i = m_peers.begin(); i != m_peers.end(); ++i)
	{
		Peer& peer = i->second;
		if (peer.state == PsEstablished && peer.errorCount == 0)
		{
			std::sort(peer.timeOffsets.begin(), peer.timeOffsets.end());
			timeOffset = std::max(timeOffset, peer.timeOffsets[peer.timeOffsets.capacity() / 2]);
			peer.timeOffsets.clear();
		}
	}

	// Adjust time; don't adjust entire offset at once as it's, at best, an approximation.
	timeOffset = (timeOffset < 30) ? timeOffset / 2 : (timeOffset * 2) / 3;
	if (timeOffset > 0)
		adjustTime(timeOffset);
}

void Replicator::dispatchEventListeners()
{
	for (std::list< EventIn >::const_iterator i = m_eventsIn.begin(); i != m_eventsIn.end(); ++i)
	{
		const EventIn& event = *i;
		for (RefArray< IListener >::iterator j = m_listeners.begin(); j != m_listeners.end(); ++j)
			(*j)->notify(this, event.time / 1000.0f, event.eventId, event.handle, event.object);
	}
	m_eventsIn.clear();
}

bool Replicator::sendIAm(handle_t peerHandle, uint8_t sequence, uint32_t id)
{
	Message msg;

	msg.type = MtIAm;
	msg.time = m_time;
	msg.iam.sequence = sequence;
	msg.iam.id = id;

	uint32_t msgSize = sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint32_t);
	return send(peerHandle, &msg, msgSize, false);
}

bool Replicator::sendBye(handle_t peerHandle)
{
	Message msg;

	msg.type = MtBye;
	msg.time = m_time;

	uint32_t msgSize = sizeof(uint8_t) + sizeof(uint32_t);
	return send(peerHandle, &msg, msgSize, true);
}

bool Replicator::sendPing(handle_t peerHandle)
{
	Message msg;

	msg.type = MtPing;
	msg.time = m_time0;

	uint32_t msgSize = sizeof(uint8_t) + sizeof(uint32_t);
	return send(peerHandle, &msg, msgSize, false);
}

bool Replicator::sendPong(handle_t peerHandle, int32_t time0)
{
	std::map< handle_t, Peer >::const_iterator i = m_peers.find(peerHandle);
	Message msg;

	msg.type = MtPong;
	msg.time = m_time0;
	msg.pong.time0 = time0;
	msg.pong.latency = (i != m_peers.end()) ? i->second.latencyMedian : 0;	// Report back my perception of latency to this peer.

	uint32_t msgSize = sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t);
	return send(peerHandle, &msg, msgSize, false);
}

void Replicator::adjustTime(int32_t offset)
{
	m_time += offset;

	// Adjust all ghost states.
	for (std::map< handle_t, Peer >::iterator i = m_peers.begin(); i != m_peers.end(); ++i)
	{
		Ghost* ghost = i->second.ghost;
		if (ghost)
		{
			ghost->Tn2 += offset;
			ghost->Tn1 += offset;
			ghost->T0 += offset;
		}
	}

	// Adjust on all queued events also.
	for (std::list< EventIn >::iterator i = m_eventsIn.begin(); i != m_eventsIn.end(); ++i)
		i->time += offset;
}

bool Replicator::send(handle_t peerHandle, const Message* msg, uint32_t size, bool reliable)
{
	T_ASSERT (size <= Message::MessageSize);
	std::map< handle_t, Peer >::const_iterator i = m_peers.find(peerHandle);
	if (i != m_peers.end())
		return m_replicatorPeers->send(peerHandle, msg, size, reliable);
	else
		return false;
}

int32_t Replicator::receive(Message* msg, handle_t& outPeerHandle)
{
	return m_replicatorPeers->receive(msg, sizeof(Message), outPeerHandle);
}

	}
}
