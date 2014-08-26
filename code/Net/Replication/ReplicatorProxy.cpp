#include "Core/Io/MemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/CompactSerializer.h"
#include "Net/Replication/INetworkTopology.h"
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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.ReplicatorProxy", ReplicatorProxy, Object)

const std::wstring& ReplicatorProxy::getName() const
{
	return m_name;
}

uint8_t ReplicatorProxy::getStatus() const
{
	return m_status;
}

double ReplicatorProxy::getLatency() const
{
	return (m_latencyMedian + m_latencyReverse) / 2.0;
}

double ReplicatorProxy::getLatencyUp() const
{
	return m_latencyMedian;
}

double ReplicatorProxy::getLatencyDown() const
{
	return m_latencyReverse;
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
}

const StateTemplate* ReplicatorProxy::getStateTemplate() const
{
	return m_stateTemplate;
}

Ref< const State > ReplicatorProxy::getState(double timeOffset) const
{
	if (m_stateTemplate)
	{
		double maxDelta = m_replicator->m_configuration.maxExtrapolationDelta;
		double delta = clamp< double >(timeOffset - m_stateTime0, -maxDelta, maxDelta);

		return m_stateTemplate->extrapolate(
			m_stateN2,
			float(m_stateTimeN2),
			m_stateN1,
			float(m_stateTimeN1),
			m_state0,
			float(m_stateTime0),
			float(m_stateTime0 + delta)
		);
	}
	else
		return 0;
}

void ReplicatorProxy::sendEvent(const ISerializable* eventObject)
{
	Event e;

	e.msg.id = RmiEvent;
	e.msg.time = time2net(m_replicator->m_time);

	e.msg.event.sequence = m_sequence++;
	
	MemoryStream ms(e.msg.event.data, RmiEvent_MaxEventSize(), false, true);
	CompactSerializer cs(&ms, &m_replicator->m_eventTypes[0], m_replicator->m_eventTypes.size());
	if (!cs.writeObject(eventObject))
	{
		log::error << m_replicator->getLogPrefix() << L"Unable to wrap event object of " << type_name(eventObject) << L"." << Endl;
		return;
	}

	cs.flush();

	e.time = m_replicator->m_time0;
	e.size = ms.tell();

	m_events.push_back(e);

	//log::info << m_replicator->getLogPrefix() << L"Sending event " << int32_t(e.msg.event.sequence) << L" " << type_name(eventObject) << L"..." << Endl;
	m_replicator->m_topology->send(m_handle, &e.msg, RmiEvent_NetSize(e.size));
}

bool ReplicatorProxy::updateEventQueue()
{
	for (std::list< Event >::iterator i = m_events.begin(); i != m_events.end(); ++i)
	{
		if (m_replicator->m_time0 - i->time > c_resendTimeThreshold)
		{
			i->time = m_replicator->m_time0;

			//log::info << m_replicator->getLogPrefix() << L"Re-sending event " << int32_t(i->msg.event.sequence) << L"..." << Endl;
			m_replicator->m_topology->send(m_handle, &i->msg, RmiEvent_NetSize(i->size));
		}
	}
	return true;
}

bool ReplicatorProxy::isEventNew(uint8_t sequence)
{
	if (m_lastEvents.find(sequence) >= 0)
		return false;

	m_lastEvents.push_back(sequence);
	return true;
}

void ReplicatorProxy::receivedEventAcknowledge(uint8_t sequence)
{
	for (std::list< Event >::iterator i = m_events.begin(); i != m_events.end(); ++i)
	{
		if (i->msg.event.sequence == sequence)
		{
			m_events.erase(i);
			break;
		}
	}
}

void ReplicatorProxy::updateLatency(double roundTrip, double latencyReverse)
{
	m_roundTrips.push_back(roundTrip);

	double sorted[17];
	for (uint32_t i = 0; i < m_roundTrips.size(); ++i)
		sorted[i] = m_roundTrips[i];

	std::sort(&sorted[0], &sorted[m_roundTrips.size()]);

	m_latencyMedian = m_roundTrips[m_roundTrips.size() / 2] / 2;
	m_latencyReverse = latencyReverse;
}

bool ReplicatorProxy::receivedState(double stateTime, const void* stateData, uint32_t stateDataSize)
{
	if (!m_stateTemplate)
		return false;

	if (stateTime < m_stateTime0)
		return false;

	Ref< const State > state = m_stateTemplate->unpack(stateData, stateDataSize);
	if (!state)
		return false;

	m_stateN2 = m_stateN1;
	m_stateTimeN2 = m_stateTimeN1;
	m_stateN1 = m_state0;
	m_stateTimeN1 = m_stateTime0;
	m_state0 = state;
	m_stateTime0 = stateTime;

	return true;
}

void ReplicatorProxy::disconnect()
{
	m_replicator = 0;
	m_handle = 0;
	m_status = 0;
	m_object = 0;
	m_sequence = 0;
	m_distance = 0.0f;
	m_timeUntilTxPing = 0.0;
	m_timeUntilTxState = 0.0;
	m_latencyMedian = 0.0;
	m_latencyReverse = 0.0;
	m_events.clear();
	m_lastEvents.clear();
}

ReplicatorProxy::ReplicatorProxy(Replicator* replicator, net_handle_t handle, const std::wstring& name)
:	m_replicator(replicator)
,	m_handle(handle)
,	m_name(name)
,	m_status(0)
,	m_origin(Transform::identity())
,	m_stateTimeN2(0.0)
,	m_stateTimeN1(0.0)
,	m_stateTime0(0.0)
,	m_sequence(0)
,	m_distance(0.0f)
,	m_timeUntilTxPing(0.0)
,	m_timeUntilTxState(0.0)
,	m_latencyMedian(0.0)
,	m_latencyReverse(0.0)
{
}

	}
}
