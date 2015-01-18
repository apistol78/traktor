#include <cstring>
#include "Core/Io/MemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Float.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/CompactSerializer.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Timer/Measure.h"
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

const double c_maxDeltaTime = 10.0;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.Replicator", Replicator, Object)

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.Replicator.IListener", Replicator::IListener, Object)

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.Replicator.IEventListener", Replicator::IEventListener, Object)

Replicator::Replicator()
:	m_time0(0.0)
,	m_time(0.0)
,	m_timeVariance(0.0)
,	m_status(0)
,	m_allowPrimaryRequests(true)
,	m_origin(Transform::identity())
,	m_sendState(false)
,	m_timeSynchronization(true)
,	m_timeSynchronized(false)
{
}

Replicator::~Replicator()
{
	destroy();
}

bool Replicator::create(INetworkTopology* topology, const Configuration& configuration)
{
	m_topology = topology;
	m_topology->setCallback(this);
	m_configuration = configuration;
	m_timer.start();
	return true;
}

void Replicator::destroy()
{
	removeAllEventTypes();
	removeAllListeners();

	if (m_topology)
	{
		m_topology->setCallback(0);
		m_topology = 0;
	}
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

void Replicator::removeAllEventTypes()
{
	m_eventTypes.resize(0);
}

Replicator::IListener* Replicator::addListener(IListener* listener)
{
	m_listeners.push_back(listener);
	return listener;
}

void Replicator::removeListener(IListener* listener)
{
	m_listeners.remove(listener);
}

void Replicator::removeAllListeners()
{
	m_listeners.clear();
}

Replicator::IEventListener* Replicator::addEventListener(const TypeInfo& eventType, IEventListener* eventListener)
{
	m_eventListeners[&eventType].push_back(eventListener);
	return eventListener;
}

void Replicator::removeEventListener(IEventListener* eventListener)
{
	for (std::map< const TypeInfo*, RefArray< IEventListener > >::iterator i = m_eventListeners.begin(); i != m_eventListeners.end(); ++i)
		i->second.remove(eventListener);
}

void Replicator::removeAllEventListeners()
{
	m_eventListeners.clear();
}

bool Replicator::update()
{
	T_MEASURE_BEGIN();

	RMessage msg;
	RMessage reply;
	net_handle_t from;

	//double T0 = m_timer.getElapsedTime();
	double dT = m_timer.getDeltaTime();
	if (dT > c_maxDeltaTime)
	{
		log::error << getLogPrefix() << L"Delta time from system clock too large; Unable to keep replicator up to date." << Endl;
		return false;
	}

	// Update underlying network topology layer.
	if (!m_topology->update(dT))
		return false;

	T_MEASURE_UNTIL(0.01);

	// Send ping to proxies.
	{
		msg.id = RmiPing;
		msg.time = time2net(m_time);
		msg.ping.time0 = time2net(m_time0);
		msg.ping.status = m_status;

		for (RefArray< ReplicatorProxy >::const_iterator i = m_proxies.begin(); i != m_proxies.end(); ++i)
		{
			if (((*i)->m_timeUntilTxPing -= dT) <= 0.0)
			{
				T_MEASURE_STATEMENT(m_topology->send((*i)->m_handle, &msg, RmiPing_NetSize()), 0.001);
				(*i)->m_timeUntilTxPing = m_configuration.timeUntilTxPing;
			}
		}
	}

	T_MEASURE_UNTIL(0.002);

	// Send our state to proxies.
	if (m_sendState && m_stateTemplate && m_state)
	{
		msg.id = RmiState;
		msg.time = time2net(m_time);
	
		uint32_t stateDataSize = 0;
		T_MEASURE_STATEMENT(stateDataSize = m_stateTemplate->pack(
			m_state,
			msg.state.data,
			RmiState_MaxStateSize()
		), 0.001);

		for (RefArray< ReplicatorProxy >::const_iterator i = m_proxies.begin(); i != m_proxies.end(); ++i)
		{
			if ((*i)->m_sendState && ((*i)->m_timeUntilTxState -= dT) <= 0.0)
			{
				T_MEASURE_STATEMENT(m_topology->send((*i)->m_handle, &msg, RmiState_NetSize(stateDataSize)), 0.001);

				Vector4 direction = (*i)->m_origin.translation() - m_origin.translation();
				Scalar distance = direction.length();

				float t = clamp((distance - m_configuration.nearDistance) / (m_configuration.farDistance - m_configuration.nearDistance), 0.0f, 1.0f);

				(*i)->m_distance = distance;
				(*i)->m_timeUntilTxState = lerp(m_configuration.timeUntilTxStateNear, m_configuration.timeUntilTxStateFar, t);
			}
		}
	}

	T_MEASURE_UNTIL(0.002);

	double timeOffset = 0.0;
	bool timeOffsetReceived = false;
	uint32_t stateIssued = 0;
	uint32_t eventsIssued = 0;

	// Receive messages.
	for (;;)
	{
		from = 0;

		// Poll message from topology.
		int32_t nrecv = 0;
		T_MEASURE_STATEMENT(nrecv = m_topology->recv(&msg, sizeof(msg), from), 0.001);
		if (nrecv <= 0)
			break;

		// Find proxy from which we received a message.
		Ref< ReplicatorProxy > fromProxy;
		for (RefArray< ReplicatorProxy >::const_iterator i = m_proxies.begin(); i != m_proxies.end(); ++i)
		{
			if ((*i)->m_handle == from)
			{
				fromProxy = *i;
				break;
			}
		}

		if (!fromProxy)
		{
			log::error << getLogPrefix() << L"Received message (" << int32_t(msg.id) << L") from unknown proxy " << from << L"; message ignored." << Endl;
			continue;
		}

		if (fromProxy->isPrimary())
		{
			double latency = fromProxy->getLatencyDown();
			double ghostOffset = net2time(msg.time) + latency - m_time;

			if (!timeOffsetReceived)
				timeOffset = ghostOffset;
			else
			{
				if (ghostOffset > 0.0f && timeOffset > 0.0f)
					timeOffset = std::max(timeOffset, ghostOffset);
				else if (ghostOffset < 0.0f && timeOffset < 0.0f)
					timeOffset = std::min(timeOffset, ghostOffset);
			}

			timeOffsetReceived = true;
		}

		if (msg.id == RmiPing)
		{
			fromProxy->m_status = msg.ping.status;

			reply.id = RmiPong;
			reply.time = time2net(m_time);
			reply.pong.time0 = msg.ping.time0;
			reply.pong.latency = time2net(fromProxy->getLatency());

			T_MEASURE_STATEMENT(m_topology->send(fromProxy->m_handle, &reply, RmiPong_NetSize()), 0.001);
		}
		else if (msg.id == RmiPong)
		{
			double pingTime = min(m_time0, net2time(msg.pong.time0));
			double roundTrip = m_time0 - pingTime;
			double reverseLatency = net2time(msg.pong.latency);

			T_MEASURE_STATEMENT(fromProxy->updateLatency(roundTrip, reverseLatency), 0.001);
		}
		else if (msg.id == RmiState)
		{
			if (fromProxy->m_stateTemplate)
			{
				bool received;
				T_MEASURE_STATEMENT(received = fromProxy->receivedState(net2time(msg.time), msg.state.data, RmiState_StateSize(nrecv)), 0.001);
				if (received)
				{
					for (RefArray< IListener >::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
					{
						T_MEASURE_STATEMENT((*i)->notify(
							this,
							float(net2time(msg.time)),
							IListener::ReState,
							fromProxy,
							fromProxy->m_state0
						), 0.001);
					}
					++stateIssued;
				}
			}
			else
			{
				log::info << getLogPrefix() << L"Received state from " << from << L" but no state template registered; state ignored." << Endl;
				for (RefArray< IListener >::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
				{
					T_MEASURE_STATEMENT((*i)->notify(
						this,
						float(net2time(msg.time)),
						IListener::ReStateError,
						fromProxy,
						0
					), 0.001);
				}
			}
		}
		else if (msg.id == RmiEvent)
		{
			// Send back event acknowledge.
			reply.id = RmiEventAck;
			reply.time = time2net(m_time);
			reply.eventAck.sequence = msg.event.sequence;
			T_MEASURE_STATEMENT(m_topology->send(fromProxy->m_handle, &reply, RmiEventAck_NetSize()), 0.001);

			// Unwrap event object.
			MemoryStream ms(msg.event.data, RmiEvent_EventSize(nrecv), true, false);
			Ref< ISerializable > eventObject;
			T_MEASURE_STATEMENT(eventObject = CompactSerializer(&ms, &m_eventTypes[0], m_eventTypes.size()).readObject< ISerializable >(), 0.001);
			if (eventObject)
			{
				// Prevent resent events from being issued into game.
				bool accept;
				T_MEASURE_STATEMENT(accept = fromProxy->acceptEvent(msg.event.sequence, eventObject), 0.001);
				if (accept)
				{
					std::map< const TypeInfo*, RefArray< IEventListener > >::const_iterator it = m_eventListeners.find(&type_of(eventObject));
					if (it != m_eventListeners.end())
					{
						for (RefArray< IEventListener >::const_iterator i = it->second.begin(); i != it->second.end(); ++i)
						{
							T_MEASURE_STATEMENT((*i)->notify(
								this,
								float(net2time(msg.time)),
								fromProxy,
								eventObject
							), 0.001);
						}
						++eventsIssued;
					}
				}
			}
			else if (!m_eventTypes.empty())
				log::error << getLogPrefix() << L"Unable to unwrap event object; size = " << RmiEvent_EventSize(nrecv) << Endl;
		}
		else if (msg.id == RmiEventAck)
		{
			// Received an event acknowledge; discard event from queue.
			T_MEASURE_STATEMENT(fromProxy->receivedEventAcknowledge(fromProxy, msg.eventAck.sequence), 0.001);
		}
	}

	T_MEASURE_UNTIL(0.004);

	// Update proxy queues.
	for (RefArray< ReplicatorProxy >::iterator i = m_proxies.begin(); i != m_proxies.end(); ++i)
		(*i)->updateEventQueue();

	T_MEASURE_UNTIL(0.001);

	if (timeOffsetReceived)
	{
		// Adjust times based from estimated time offset.
		if (abs(timeOffset) >= 1.0)
		{
			m_timeSynchronized = false;
			timeOffset *= 0.8;
		}
		else if (abs(timeOffset) > 0.06)
		{
			m_timeSynchronized = false;
			timeOffset *= 0.4;
		}
		else
		{
			double k = abs(timeOffset) / 0.06;
			timeOffset *= 0.4 * k;
		}

		if (m_timeSynchronization || !m_timeSynchronized)
		{
			if (abs(timeOffset) > 0.01)
			{
				m_time += timeOffset;
				for (RefArray< ReplicatorProxy >::iterator i = m_proxies.begin(); i != m_proxies.end(); ++i)
				{
					(*i)->m_stateTimeN2 += timeOffset;
					(*i)->m_stateTimeN1 += timeOffset;
					(*i)->m_stateTime0 += timeOffset;
				}

				for (RefArray< IListener >::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
				{
					T_MEASURE_STATEMENT((*i)->notify(
						this,
						float(timeOffset),
						IListener::ReTimeAdjust,
						0,
						0
					), 0.001);
				}
			}
		}

		// Update time variance; this should become fairly stable after a couple of estimates.
		m_timeErrors.push_back(abs(timeOffset));
		{
			double k = 0.0;
			for (uint32_t i = 0; i < m_timeErrors.size(); ++i)
				k += m_timeErrors[i];
			k /= double(m_timeErrors.size());
			m_timeVariance = std::sqrt((k * k) / 6.0);

			// Have we reached acceptable variance?
			if (!m_timeSynchronized && m_timeVariance <= 0.03 && abs(timeOffset) <= 0.01)
			{
				if (!m_timeSynchronization)
					log::info << getLogPrefix() << L"Time re-synchronized" << Endl;
				m_timeSynchronized = true;
			}
		}
	}

	T_MEASURE_UNTIL(0.001);

	// Need to migrate primary if anyone is "in session" before I.
	if ((m_status & 0x80) == 0x00 && isPrimary())
	{
		for (RefArray< ReplicatorProxy >::iterator i = m_proxies.begin(); i != m_proxies.end(); ++i)
		{
			if (((*i)->getStatus() & 0x80) == 0x80)
			{
				if ((*i)->setPrimary())
				{
					log::info << getLogPrefix() << L"Migrated primary token to peer " << (*i)->getHandle() << L"." << Endl;
					break;
				}
				else
					log::info << getLogPrefix() << L"Unable migrate primary token to peer " << (*i)->getHandle() << L"." << Endl;
			}
		}
	}

	T_MEASURE_UNTIL(0.001);

	m_time += dT;
	m_time0 += dT;

	return true;
}

const std::wstring& Replicator::getName() const
{
	return m_name;
}

void Replicator::setStatus(uint8_t status)
{
	if (m_status != status)
	{
		// If status has changed we need to ping our
		// fellow peers as soon as possible to let them
		// know about our new status.
		for (RefArray< ReplicatorProxy >::const_iterator i = m_proxies.begin(); i != m_proxies.end(); ++i)
			(*i)->m_timeUntilTxPing = 0.0;

		m_status = status;
	}
}

uint8_t Replicator::getStatus() const
{
	return m_status;
}

bool Replicator::isPrimary() const
{
	return m_topology->getPrimaryHandle() == m_topology->getLocalHandle();
}

void Replicator::setOrigin(const Transform& origin)
{
	m_origin = origin;
}

void Replicator::setStateTemplate(const StateTemplate* stateTemplate)
{
	m_stateTemplate = stateTemplate;
}

void Replicator::setState(const State* state)
{
	// If state represent a radical change we need to send
	// the state to fellow peers as soon as possible.
	if (state != 0 && m_stateTemplate->critical(m_state, state))
	{
		for (RefArray< ReplicatorProxy >::const_iterator i = m_proxies.begin(); i != m_proxies.end(); ++i)
		{
			if ((*i)->m_distance < m_configuration.furthestDistance)
				(*i)->m_timeUntilTxState = 0.0;
		}
	}
	m_state = state;
}

const State* Replicator::getState() const
{
	return m_state;
}

void Replicator::setSendState(bool sendState)
{
	m_sendState = sendState;
}

void Replicator::setTimeSynchronization(bool timeSynchronization)
{
	m_timeSynchronization = timeSynchronization;
}

uint32_t Replicator::getProxyCount() const
{
	return m_proxies.size();
}

ReplicatorProxy* Replicator::getProxy(uint32_t index) const
{
	return m_proxies[index];
}

bool Replicator::broadcastEvent(const ISerializable* eventObject)
{
	for (RefArray< ReplicatorProxy >::const_iterator i = m_proxies.begin(); i != m_proxies.end(); ++i)
		(*i)->sendEvent(eventObject);
	return true;
}

ReplicatorProxy* Replicator::getPrimaryProxy() const
{
	for (RefArray< ReplicatorProxy >::const_iterator i = m_proxies.begin(); i != m_proxies.end(); ++i)
	{
		if ((*i)->isPrimary())
			return *i;
	}
	return 0;
}

double Replicator::getAverageLatency() const
{
	double latency = 0.0;
	if (!m_proxies.empty())
	{
		for (RefArray< ReplicatorProxy >::const_iterator i = m_proxies.begin(); i != m_proxies.end(); ++i)
			latency += (*i)->getLatency();
		latency /= double(m_proxies.size());
	}
	return latency;
}

bool Replicator::sendEventToPrimary(const ISerializable* eventObject)
{
	if (!isPrimary())
	{
		// Find primary proxy and send event to it.
		ReplicatorProxy* primaryProxy = getPrimaryProxy();
		if (primaryProxy)
		{
			primaryProxy->sendEvent(eventObject);
			return true;
		}
		else
		{
			log::error << getLogPrefix() << L"Unable to send event " << type_name(eventObject) << L" to primary; no primary found." << Endl;
			return false;
		}
	}
	else
	{
		// We are primary peer; dispatch event directly.
		std::map< const TypeInfo*, RefArray< IEventListener > >::const_iterator it = m_eventListeners.find(&type_of(eventObject));
		if (it != m_eventListeners.end())
		{
			for (RefArray< IEventListener >::const_iterator i = it->second.begin(); i != it->second.end(); ++i)
			{
				(*i)->notify(
					this,
					float(m_time),
					0,
					eventObject
				);
			}
		}
		return true;
	}
}

double Replicator::getTime() const
{
	return m_time;
}

double Replicator::getTimeVariance() const
{
	return m_timeVariance;
}

std::wstring Replicator::getLogPrefix() const
{
	return L"Replicator: [" + toString(m_topology->getLocalHandle()) + L"] ";
}

bool Replicator::nodeConnected(INetworkTopology* topology, net_handle_t node)
{
	std::wstring name;

	for (int32_t i = 0; i < topology->getNodeCount(); ++i)
	{
		if (topology->getNodeHandle(i) == node)
		{
			name = topology->getNodeName(i);
			break;
		}
	}

	if (node != m_topology->getLocalHandle())
	{
		Ref< ReplicatorProxy > proxy = new ReplicatorProxy(this, node, name);
		m_proxies.push_back(proxy);

		log::info << getLogPrefix() << L"Proxy for node " << node << L" (" << name << L") created." << Endl;

		for (RefArray< IListener >::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
		{
			(*i)->notify(
				this,
				float(m_time),
				IListener::ReConnected,
				proxy,
				0
			);
		}
	}
	else
	{
		m_name = name;
	}

	return true;
}

bool Replicator::nodeDisconnected(INetworkTopology* topology, net_handle_t node)
{
	for (RefArray< ReplicatorProxy >::iterator i = m_proxies.begin(); i != m_proxies.end(); ++i)
	{
		if ((*i)->m_handle == node)
		{
			log::info << getLogPrefix() << L"Proxy for node " << node << L" (" << (*i)->getName() << L") destroyed." << Endl;
			
			for (RefArray< IListener >::const_iterator j = m_listeners.begin(); j != m_listeners.end(); ++j)
			{
				(*j)->notify(
					this,
					float(m_time),
					IListener::ReDisconnected,
					*i,
					0
				);
			}

			(*i)->disconnect();
			m_proxies.erase(i);

			break;
		}
	}
	return true;
}

	}
}
