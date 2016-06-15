#include "Core/Io/MemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/CompactSerializer.h"
#include "Core/Serialization/DeepHash.h"
#include "Net/Replication/INetworkTopology.h"
#include "Net/Replication/IReplicatorEventListener.h"
#include "Net/Replication/Replicator.h"
#include "Net/Replication/ReplicatorProxy.h"
#include "Net/Replication/ReplicatorTypes.h"
#include "Net/Replication/State/State.h"
#include "Net/Replication/State/StateTemplate.h"

namespace traktor
{
	namespace net
	{
		namespace
		{

const double c_resendTimeThreshold = 0.5;
const int32_t c_resendCountThreshold = 16;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.ReplicatorProxy", ReplicatorProxy, Object)

net_handle_t ReplicatorProxy::getHandle() const
{
	return m_handle;
}

const std::wstring& ReplicatorProxy::getName() const
{
	return m_name;
}

Object* ReplicatorProxy::getUser() const
{
	return m_user;
}

uint8_t ReplicatorProxy::getStatus() const
{
	return m_status;
}

bool ReplicatorProxy::isLatencyReliable() const
{
	return m_roundTrips.full();
}

double ReplicatorProxy::getLatency() const
{
	return m_latency;
}

double ReplicatorProxy::getLatencySpread() const
{
	return m_latencyStandardDeviation;
}

double ReplicatorProxy::getReverseLatency() const
{
	return m_latencyReverse;
}

double ReplicatorProxy::getReverseLatencySpread() const
{
	return m_latencyReverseStandardDeviation;
}

void ReplicatorProxy::resetLatencies()
{
	m_remoteTimes.clear();
	m_roundTrips.clear();
	m_latency = 0.0;
	m_latencyStandardDeviation = 0.0;
	m_latencyReverse = 0.0;
	m_latencyReverseStandardDeviation = 0.0;
}

double ReplicatorProxy::getTimeRate() const
{
	return m_timeRate;
}

bool ReplicatorProxy::isConnected() const
{
	return m_handle != 0;
}

bool ReplicatorProxy::setPrimary()
{
	if (m_replicator)
		return m_replicator->m_topology->setPrimaryHandle(m_handle);
	else
		return false;
}

bool ReplicatorProxy::isPrimary() const
{
	if (m_replicator)
		return m_replicator->m_topology->getPrimaryHandle() == m_handle;
	else
		return false;
}

bool ReplicatorProxy::isRelayed() const
{
	if (m_replicator)
	{
		int32_t nodeCount = m_replicator->m_topology->getNodeCount();
		for (int32_t i = 0; i < nodeCount; ++i)
		{
			if (m_replicator->m_topology->getNodeHandle(i) == m_handle)
				return m_replicator->m_topology->isNodeRelayed(i);
		}
		return false;
	}
	else
		return false;
}

void ReplicatorProxy::setObject(Object* object)
{
	m_object = object;
}

Object* ReplicatorProxy::getObject() const
{
	return m_object;
}

void ReplicatorProxy::setOrigin(const Transform& origin)
{
	m_origin = origin;
}

const Transform& ReplicatorProxy::getOrigin() const
{
	return m_origin;
}

void ReplicatorProxy::setStateTemplate(const StateTemplate* stateTemplate)
{
	m_stateTemplate = stateTemplate;

	// Old states must be immediately discarded; we cannot keep
	// states produced from old template.
	resetStates();
}

const StateTemplate* ReplicatorProxy::getStateTemplate() const
{
	return m_stateTemplate;
}

Ref< const State > ReplicatorProxy::getState(double time, double limit) const
{
	if (m_stateTemplate)
	{
		double k = m_stateReceivedTime - m_stateTime0;
		double offset = std::max(k - limit, 0.0);

		return m_stateTemplate->extrapolate(
			m_stateN2,
			float(m_stateTimeN2 + offset),
			m_stateN1,
			float(m_stateTimeN1 + offset),
			m_state0,
			float(m_stateTime0 + offset),
			float(time)
		);
	}
	else
		return 0;
}

Ref< const State > ReplicatorProxy::getFilteredState(double time, double limit, const State* currentState, float filterCoeff) const
{
	if (m_stateTemplate)
	{
		double k = m_stateReceivedTime - m_stateTime0;
		double offset = std::max(k - limit, 0.0);

		Ref< const State > extrapolatedState = m_stateTemplate->extrapolate(
			m_stateN2,
			float(m_stateTimeN2 + offset),
			m_stateN1,
			float(m_stateTimeN1 + offset),
			m_state0,
			float(m_stateTime0 + offset),
			float(time)
		);
		if (!extrapolatedState)
			return 0;

		if (currentState)
			return m_stateTemplate->extrapolate(
				0, 0.0f,
				extrapolatedState, 0.0f,
				currentState, 1.0f,
				filterCoeff
			);
		else
			return extrapolatedState;
	}
	else
		return 0;
}

void ReplicatorProxy::resetStates()
{
	m_state0 = 0;
	m_stateTime0 = 0.0;
	m_stateN1 = 0;
	m_stateTimeN1 = 0.0;
	m_stateN2 = 0;
	m_stateTimeN2 = 0.0;
}

void ReplicatorProxy::setSendState(bool sendState)
{
	m_sendState = sendState;
}

void ReplicatorProxy::sendEvent(const ISerializable* eventObject, bool inOrder)
{
	T_FATAL_ASSERT (eventObject);

	if (!m_replicator)
		return;

	// Pack event structure.
	TxEvent e;
	e.msg.id = inOrder ? RmiEvent1 : RmiEvent0;
	e.msg.time = time2net(m_replicator->m_time);
	e.msg.event.sequence = inOrder ? m_txSequenceInOrder++ : m_txSequence++;

	MemoryStream ms(e.msg.event.data, RmiEvent_MaxEventSize(), false, true);
	CompactSerializer cs(&ms, &m_replicator->m_eventTypes[0], m_replicator->m_eventTypes.size());
	if (!cs.writeObject(eventObject))
	{
		log::error << m_replicator->getLogPrefix() << L"Unable to wrap event object of " << type_name(eventObject) << L"." << Endl;
		return;
	}
	cs.flush();

	e.size = ms.tell();
	e.time = 0.0;
	e.count = 0;

	// Put event into outgoing queue.
	m_txEvents.push_back(e);
}

int32_t ReplicatorProxy::updateTxEventQueue()
{
	int32_t discarded = 0;

	// First send is prioritized over re-sends.
	for (std::list< TxEvent >::iterator i = m_txEvents.begin(); i != m_txEvents.end(); ++i)
	{
		if (i->count <= 0)
		{
			m_replicator->m_topology->send(m_handle, &i->msg, RmiEvent_NetSize(i->size));
			i->time = m_replicator->m_time0;
			i->count = 1;
		}
	}

	// Re-send events if no ack has been received within threshold.
	for (std::list< TxEvent >::iterator i = m_txEvents.begin(); i != m_txEvents.end(); )
	{
		if (m_replicator->m_time0 - i->time > c_resendTimeThreshold)
		{
			T_DEBUG(L"No ack received, resending event " << int32_t(i->msg.event.sequence) << L"...");

			m_replicator->m_topology->send(m_handle, &i->msg, RmiEvent_NetSize(i->size));

			i->time = m_replicator->m_time0;
			i->count++;

			// Only permitted to discard events which are NOT in order, other
			// events must keep on resending until acknowledged or connection terminates.
			if (i->msg.id == RmiEvent0 && i->count >= c_resendCountThreshold)
			{
				i = m_txEvents.erase(i);
				++discarded;
				continue;
			}
		}
		++i;
	}

	return discarded;
}

bool ReplicatorProxy::receivedTxEventAcknowledge(const ReplicatorProxy* from, uint8_t sequence, bool inOrder)
{
	for (std::list< TxEvent >::iterator i = m_txEvents.begin(); i != m_txEvents.end(); ++i)
	{
		if (
			((inOrder && i->msg.id == RmiEvent1) || (!inOrder && i->msg.id == RmiEvent0)) &&
			i->msg.event.sequence == sequence
		)
		{
			if (i->count >= 2)
				T_DEBUG(L"Resent event " << int32_t(i->msg.event.sequence) << L" acknowledged");
			m_txEvents.erase(i);
			return true;
		}
	}
	return false;
}

bool ReplicatorProxy::receivedRxEvent(uint32_t time, uint8_t sequence, const ISerializable* eventObject, bool inOrder)
{
	if (!eventObject)
		return false;

	// Discard duplicated events.
	CircularVector< uint8_t, 64 >& rxEventSequences = inOrder ? m_rxEventSequences[1] : m_rxEventSequences[0];
	if (rxEventSequences.find(sequence) >= 0)
	{
		T_DEBUG(L"Discarded event " << int32_t(sequence) << L", already received");
		return true;
	}
	rxEventSequences.push_back(sequence);

	// Enqueue events, special care taken for in-order events.
	RxEvent e;
	e.time = time;
	e.sequence = sequence;
	e.eventObject = eventObject;

	if (inOrder)
	{
		T_DEBUG(L"Received in-order event " << int32_t(sequence) << L", waiting for " << int32_t(m_rxEventsInOrderSequence));
		m_rxEventsInOrderQueue[sequence] = e;
	}
	else
		m_rxEvents.push_back(e);

	return true;
}

bool ReplicatorProxy::dispatchRxEvents(const SmallMap< const TypeInfo*, RefArray< IReplicatorEventListener > >& eventListeners)
{
	// Move as many "in order" events into dispatch queue as possible.
	while (m_rxEventsInOrderQueue[m_rxEventsInOrderSequence].eventObject)
	{
		T_DEBUG(L"In-order event consumed " << int32_t(m_rxEventsInOrderSequence));
		m_rxEvents.push_back(m_rxEventsInOrderQueue[m_rxEventsInOrderSequence]);
		m_rxEventsInOrderQueue[m_rxEventsInOrderSequence].eventObject = 0;
		m_rxEventsInOrderSequence++;
	}

	// Dispatch received events.
	for (AlignedVector< RxEvent >::const_iterator i = m_rxEvents.begin(); i != m_rxEvents.end(); ++i)
	{
		T_FATAL_ASSERT (i->eventObject);
		bool processed = false;
		uint32_t count = 0;

		SmallMap< const TypeInfo*, RefArray< IReplicatorEventListener > >::const_iterator it = eventListeners.find(&type_of(i->eventObject));
		if (it != eventListeners.end())
		{
			T_ANONYMOUS_VAR(Ref< const ISerializable >)(i->eventObject);
			for (RefArray< IReplicatorEventListener >::const_iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
			{
				processed |= (*it2)->notify(
					m_replicator,
					net2time(i->time),
					this,
					i->eventObject
				);
				++count;
			}
		}

		if (!processed && count > 0)
			log::warning << m_replicator->getLogPrefix() << L"Event " << type_name(i->eventObject) << L" from " << getLogIdentifier() << L" not processed by " << count << L" listener(s); event discarded." << Endl;
	}

	m_rxEvents.resize(0);
	return true;
}

void ReplicatorProxy::updateLatency(double localTime, double remoteTime, double roundTrip, double latencyReverse, double latencyReverseSpread)
{
	// Keep buffer of remote times, also calculate "rate of time" of remote proxy.
	if (m_remoteTimes.empty() || localTime - m_remoteTimes.back().first > FUZZY_EPSILON)
		m_remoteTimes.push_back(std::make_pair(localTime, remoteTime));
	if (m_remoteTimes.size() >= 16)
	{
		m_timeRate = 0.0;
		for (uint32_t i = 0; i < m_remoteTimes.size() - 1; ++i)
		{
			double dLocalT = m_remoteTimes[i + 1].first - m_remoteTimes[i].first;
			double dRemoteT = m_remoteTimes[i + 1].second - m_remoteTimes[i].second;
			m_timeRate += dRemoteT / dLocalT;
		}
		m_timeRate /= double(m_remoteTimes.size() - 1);
	}

	// Keep circular buffer of latest round trips.
	m_roundTrips.push_back(roundTrip);

	double denom = 1.0 / double(m_roundTrips.size());

	// Calculate average latency.
	double sum = 0;
	for (uint32_t i = 0; i < m_roundTrips.size(); ++i)
		sum += m_roundTrips[i];
	m_latency = (sum / 2.0) * denom;

	// Calculate standard deviation.
	double sumDiff = 0;
	for (uint32_t i = 0; i < m_roundTrips.size(); ++i)
	{
		double diff = m_roundTrips[i] / 2.0 - m_latency;
		sumDiff += diff * diff;
	}
	m_latencyStandardDeviation = std::sqrt(sumDiff * denom);

	// Save reverse metrics.
	m_latencyReverse = latencyReverse;
	m_latencyReverseStandardDeviation = latencyReverseSpread;
}

bool ReplicatorProxy::receivedState(double localTime, double stateTime, const void* stateData, uint32_t stateDataSize)
{
	if (!m_stateTemplate)
	{
		log::info << m_replicator->getLogPrefix() << L"Received state (" << stateDataSize << L" byte(s)) from " << getLogIdentifier() << L" but no state template registered; state ignored." << Endl;
		return false;
	}

	Ref< const State > state = m_stateTemplate->unpack(stateData, stateDataSize);
	if (!state)
	{
		log::info << m_replicator->getLogPrefix() << L"Failed to unpack state (" << stateDataSize << L" byte(s)) from " << getLogIdentifier() << L"; state ignored." << Endl;
		return false;
	}

	m_stateReceivedTime = localTime;

	if (stateTime >= m_stateTime0)
	{
		m_stateN2 = m_stateN1;
		m_stateTimeN2 = m_stateTimeN1;
		m_stateN1 = m_state0;
		m_stateTimeN1 = m_stateTime0;
		m_state0 = state;
		m_stateTime0 = stateTime;
	}
	else if (stateTime >= m_stateTimeN1)
	{
		m_stateN2 = m_stateN1;
		m_stateTimeN2 = m_stateTimeN1;
		m_stateN1 = state;
		m_stateTimeN1 = stateTime;
	}
	else if (stateTime >= m_stateTimeN2)
	{
		m_stateN2 = state;
		m_stateTimeN2 = stateTime;
	}
	else
	{
		log::info << m_replicator->getLogPrefix() << L"Received old state (" << int32_t((m_stateTimeN2 - stateTime) * 1000.0) << L" ms) from " << getLogIdentifier() << L"; state ignored." << Endl;
		return false;
	}

	return true;
}

void ReplicatorProxy::disconnect()
{
	m_replicator = 0;
	m_handle = 0;
	m_user = 0;
	m_status = 0;
	m_object = 0;
	m_distance = 0.0f;
	m_sendState = false;
	m_issueStateListeners = false;
	m_timeUntilTxPing = 0.0;
	m_timeUntilTxState = 0.0;
	m_latency = 0.0;
	m_latencyStandardDeviation = 0.0;
	m_latencyReverse = 0.0;
	m_latencyReverseStandardDeviation = 0.0;

	m_txSequence = 0;
	m_txSequenceInOrder = 0;
	m_txEvents.clear();

	for (uint32_t i = 0; i < sizeof_array(m_rxEventsInOrderQueue); ++i)
	{
		m_rxEventsInOrderQueue[i].time = 0;
		m_rxEventsInOrderQueue[i].eventObject = 0;
	}
	m_rxEventsInOrderSequence = 0;
	m_rxEvents.clear();
}

std::wstring ReplicatorProxy::getLogIdentifier() const
{
	return L"[" + toString(m_handle) + (isRelayed() ? L" (relayed)]" : L"]");
}

ReplicatorProxy::ReplicatorProxy(Replicator* replicator, net_handle_t handle, const std::wstring& name, Object* user)
:	m_replicator(replicator)
,	m_handle(handle)
,	m_name(name)
,	m_user(user)
,	m_status(0)
,	m_origin(Transform::identity())
,	m_distance(0.0f)
,	m_sendState(false)
,	m_issueStateListeners(false)
,	m_stateTimeN2(0.0)
,	m_stateTimeN1(0.0)
,	m_stateTime0(0.0)
,	m_stateReceivedTime(0.0)
,	m_txSequence(0)
,	m_txSequenceInOrder(0)
,	m_rxEventsInOrderSequence(0)
,	m_timeUntilTxPing(0.0)
,	m_timeUntilTxState(0.0)
,	m_timeRate(0.0)
,	m_latency(0.0)
,	m_latencyStandardDeviation(0.0)
,	m_latencyReverse(0.0)
,	m_latencyReverseStandardDeviation(0.0)
{
	for (uint32_t i = 0; i < sizeof_array(m_rxEventsInOrderQueue); ++i)
	{
		m_rxEventsInOrderQueue[i].time = 0;
		m_rxEventsInOrderQueue[i].eventObject = 0;
	}
}

	}
}
