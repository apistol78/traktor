#include "Core/Io/MemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Float.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/CompactSerializer.h"
#include "Core/Serialization/DeepHash.h"
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

const double c_maxDeltaTime = 0.1f;
		
		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.Replicator", Replicator, Object)

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.Replicator.IListener", Replicator::IListener, Object)

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.Replicator.IEventListener", Replicator::IEventListener, Object)

Replicator::Replicator()
:	m_time0(0.0)
,	m_time(0.0)
,	m_status(0)
,	m_allowPrimaryRequests(true)
,	m_origin(Transform::identity())
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
	RMessage msg;
	RMessage reply;
	net_handle_t from;

	double dT = std::min(m_timer.getDeltaTime(), c_maxDeltaTime);

	// Update underlying network topology layer.
	if (!m_topology->update(dT))
		return false;

	// Send ping to proxies.
	{
		msg.id = RmiPing;
		msg.time = time2net(m_time0);
		msg.ping.status = m_status;

		for (RefArray< ReplicatorProxy >::const_iterator i = m_proxies.begin(); i != m_proxies.end(); ++i)
		{
			if (((*i)->m_timeUntilTxPing -= dT) <= 0.0)
			{
				m_topology->send((*i)->m_handle, &msg, RmiPing_NetSize());
				(*i)->m_timeUntilTxPing = m_configuration.timeUntilTxPing;
			}
		}
	}

	// Send our state to proxies.
	if (m_stateTemplate && m_state)
	{
		msg.id = RmiState;
		msg.time = time2net(m_time);
	
		uint32_t stateDataSize = m_stateTemplate->pack(
			m_state,
			msg.state.data,
			RmiState_MaxStateSize()
		);

		for (RefArray< ReplicatorProxy >::const_iterator i = m_proxies.begin(); i != m_proxies.end(); ++i)
		{
			if (((*i)->m_timeUntilTxState -= dT) <= 0.0)
			{
				m_topology->send((*i)->m_handle, &msg, RmiState_NetSize(stateDataSize));

				Vector4 direction = (*i)->m_origin.translation() - m_origin.translation();
				Scalar distance = direction.length();

				float t = clamp((distance - m_configuration.nearDistance) / (m_configuration.farDistance - m_configuration.nearDistance), 0.0f, 1.0f);

				(*i)->m_distance = distance;
				(*i)->m_timeUntilTxState = lerp(m_configuration.timeUntilTxStateNear, m_configuration.timeUntilTxStateFar, t);
			}
		}
	}

	double timeOffset = 0.0;

	// Receive messages.
	for (;;)
	{
		int32_t nrecv = m_topology->recv(&msg, sizeof(msg), from);
		if (nrecv <= 0)
			break;

		Ref< ReplicatorProxy > fromGhost;
		for (RefArray< ReplicatorProxy >::const_iterator i = m_proxies.begin(); i != m_proxies.end(); ++i)
		{
			if ((*i)->m_handle == from)
			{
				fromGhost = *i;
				break;
			}
		}

		if (!fromGhost)
		{
			log::error << getLogPrefix() << L"Received message from unknown proxy " << from << L"; message ignored." << Endl;
			continue;
		}

		if (msg.id == RmiPing)
		{
			fromGhost->m_status = msg.ping.status;

			reply.id = RmiPong;
			reply.time = time2net(m_time0);
			reply.pong.time0 = msg.time;
			reply.pong.latency = time2net(fromGhost->getLatency());

			m_topology->send(fromGhost->m_handle, &reply, RmiPong_NetSize());
		}
		else if (msg.id == RmiPong)
		{
			double pingTime = min(m_time0, net2time(msg.pong.time0));
			double roundTrip = m_time0 - pingTime;
			double reverseLatency = net2time(msg.pong.latency);

			fromGhost->updateLatency(roundTrip, reverseLatency);
		}
		else if (msg.id == RmiState)
		{
			if (fromGhost->receivedState(net2time(msg.time), msg.state.data, RmiState_StateSize(nrecv)))
			{
				double latency = fromGhost->getLatency();
				timeOffset = std::max(
					net2time(msg.time) + latency - m_time,
					timeOffset
				);

				for (RefArray< IListener >::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
				{
					(*i)->notify(
						this,
						float(net2time(msg.time)),
						IListener::ReState,
						fromGhost,
						fromGhost->m_state0
					);
				}
			}
		}
		else if (msg.id == RmiEvent)
		{
			double latency = fromGhost->getLatency();
			timeOffset = std::max(
				net2time(msg.time) + latency - m_time,
				timeOffset
			);

			// Send back event acknowledge.
			reply.id = RmiEventAck;
			reply.time = time2net(m_time);
			reply.eventAck.sequence = msg.event.sequence;
			m_topology->send(fromGhost->m_handle, &reply, RmiEventAck_NetSize());

			// Unwrap event object.
			MemoryStream ms(msg.event.data, RmiEvent_EventSize(nrecv), true, false);
			Ref< ISerializable > eventObject = CompactSerializer(&ms, &m_eventTypes[0], m_eventTypes.size()).readObject< ISerializable >();
			if (eventObject)
			{
				// Prevent resent events from being issued into game.
				if (fromGhost->isEventNew(msg.event.sequence))
				{
					std::map< const TypeInfo*, RefArray< IEventListener > >::const_iterator it = m_eventListeners.find(&type_of(eventObject));
					if (it != m_eventListeners.end())
					{
						log::info << getLogPrefix() << L"Dispatching event " << type_name(eventObject) << L" to " << uint32_t(it->second.size()) << L" listener(s)..." << Endl;
						log::info << getLogPrefix() << L"\t  sender " << fromGhost->m_handle << Endl;
						log::info << getLogPrefix() << L"\tsequence " << int32_t(msg.event.sequence) << Endl;
						log::info << getLogPrefix() << L"\t    hash " << DeepHash(eventObject).get() << Endl;

						for (RefArray< IEventListener >::const_iterator i = it->second.begin(); i != it->second.end(); ++i)
						{
							(*i)->notify(
								this,
								float(net2time(msg.time)),
								fromGhost,
								eventObject
							);
						}
					}
					else
					{
						log::info << getLogPrefix() << L"Discarding event " << type_name(eventObject) << L", no listeners." << Endl;
						log::info << getLogPrefix() << L"\t  sender " << fromGhost->m_handle << Endl;
						log::info << getLogPrefix() << L"\tsequence " << int32_t(msg.event.sequence) << Endl;
						log::info << getLogPrefix() << L"\t    hash " << DeepHash(eventObject).get() << Endl;
					}
				}
				else
				{
					log::info << getLogPrefix() << L"Discarding duplicated event " << type_name(eventObject) << Endl;
					log::info << getLogPrefix() << L"\t  sender " << fromGhost->m_handle << Endl;
					log::info << getLogPrefix() << L"\tsequence " << int32_t(msg.event.sequence) << Endl;
					log::info << getLogPrefix() << L"\t    hash " << DeepHash(eventObject).get() << Endl;
				}
			}
			else if (!m_eventTypes.empty())
				log::error << getLogPrefix() << L"Unable to unwrap event object; size = " << RmiEvent_EventSize(nrecv) << Endl;
		}
		else if (msg.id == RmiEventAck)
		{
			double latency = fromGhost->getLatency();
			timeOffset = std::max(
				net2time(msg.time) + latency - m_time,
				timeOffset
			);

			// Received an event acknowledge; discard event from queue.
			//log::info << getLogPrefix() << L"Received acknowledge of event " << int32_t(msg.eventAck.sequence) << Endl;
			fromGhost->receivedEventAcknowledge(msg.eventAck.sequence);
		}
	}

	// Update proxy queues.
	for (RefArray< ReplicatorProxy >::iterator i = m_proxies.begin(); i != m_proxies.end(); ++i)
		(*i)->updateEventQueue();

	// Adjust times based from estimated time offset.
	timeOffset *= 0.8;
	if (abs(timeOffset) < 0.03)
	{
		double k = abs(timeOffset) / 0.03;
		timeOffset *= k;
	}
	if (timeOffset > 0.01)
	{
		m_time += timeOffset;
		for (RefArray< ReplicatorProxy >::iterator i = m_proxies.begin(); i != m_proxies.end(); ++i)
		{
			(*i)->m_stateTimeN2 += timeOffset;
			(*i)->m_stateTimeN1 += timeOffset;
			(*i)->m_stateTime0 += timeOffset;
		}
	}

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
				m_time,
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
					m_time,
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
