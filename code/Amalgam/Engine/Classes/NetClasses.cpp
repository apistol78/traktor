#include "Core/Io/IStream.h"
#include "Online/ILobby.h"
#include "Online/ISessionManager.h"
#include "Net/Replication/INetworkTopology.h"
#include "Net/Replication/IPeer2PeerProvider.h"
#include "Net/Replication/Peer2PeerTopology.h"
#include "Net/Replication/Replicator.h"
#include "Net/Replication/ReplicatorProxy.h"
#include "Net/Replication/State/State.h"
#include "Net/Replication/State/StateTemplate.h"
#include "Script/AutoScriptClass.h"
#include "Script/Boxes.h"
#include "Script/IScriptDelegate.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

class ReplicatorConfiguration : public Object
{
	T_RTTI_CLASS;

public:
	ReplicatorConfiguration()
	{
	}

	ReplicatorConfiguration(const net::Replicator::Configuration& configuration)
	:	m_configuration(configuration)
	{
	}

	void setNearDistance(float nearDistance) { m_configuration.nearDistance = nearDistance; }

	float getNearDistance() const { return m_configuration.nearDistance; }

	void setFarDistance(float farDistance) { m_configuration.farDistance = farDistance; }

	float getFarDistance() const { return m_configuration.farDistance; }

	void setFurthestDistance(float furthestDistance) { m_configuration.furthestDistance = furthestDistance; }

	float getFurthestDistance() const { return m_configuration.furthestDistance; }

	void setTimeUntilTxStateNear(float timeUntilTxStateNear) { m_configuration.timeUntilTxStateNear = timeUntilTxStateNear; }

	float getTimeUntilTxStateNear() const { return m_configuration.timeUntilTxStateNear; }

	void setTimeUntilTxStateFar(float timeUntilTxStateFar) { m_configuration.timeUntilTxStateFar = timeUntilTxStateFar; }

	float getTimeUntilTxStateFar() const { return m_configuration.timeUntilTxStateFar; }

	void setTimeUntilTxPing(float timeUntilTxPing) { m_configuration.timeUntilTxPing = timeUntilTxPing; }

	float getTimeUntilTxPing() const { return m_configuration.timeUntilTxPing; }

	const net::Replicator::Configuration& getConfiguration() const { return m_configuration; }

private:
	net::Replicator::Configuration m_configuration;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.ReplicatorConfiguration", ReplicatorConfiguration, Object)

class ReplicatorListener : public net::Replicator::IListener
{
	T_RTTI_CLASS;

public:
	ReplicatorListener(script::IScriptDelegate* delegate)
	:	m_delegate(delegate)
	{
	}

	virtual void notify(net::Replicator* replicator, float eventTime, uint32_t eventId, net::ReplicatorProxy* proxy, const Object* eventObject)
	{
		script::Any argv[] =
		{
			script::CastAny< Object* >::set(replicator),
			script::CastAny< float >::set(eventTime),
			script::CastAny< int32_t >::set(int32_t(eventId)),
			script::CastAny< Object* >::set(proxy),
			script::CastAny< Object* >::set((Object*)eventObject)
		};
		if (m_delegate)
			m_delegate->call(sizeof_array(argv), argv);
	}

private:
	Ref< script::IScriptDelegate > m_delegate;
};

class ReplicatorEventListener : public net::Replicator::IEventListener
{
	T_RTTI_CLASS;

public:
	ReplicatorEventListener(script::IScriptDelegate* delegate)
	:	m_delegate(delegate)
	{
	}

	virtual void notify(net::Replicator* replicator, float eventTime, net::ReplicatorProxy* fromProxy, const Object* eventObject)
	{
		script::Any argv[] =
		{
			script::CastAny< Object* >::set(replicator),
			script::CastAny< float >::set(eventTime),
			script::CastAny< Object* >::set(fromProxy),
			script::CastAny< Object* >::set((Object*)eventObject)
		};
		if (m_delegate)
			m_delegate->call(sizeof_array(argv), argv);
	}

private:
	Ref< script::IScriptDelegate > m_delegate;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.ReplicatorListener", ReplicatorListener, net::Replicator::IListener)

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.ReplicatorEventListener", ReplicatorEventListener, net::Replicator::IEventListener)

bool net_Replicator_create(net::Replicator* self, net::INetworkTopology* topology, const ReplicatorConfiguration* configuration)
{
	return self->create(topology, configuration ? configuration->getConfiguration() : net::Replicator::Configuration());
}

void net_Replicator_setConfiguration(net::Replicator* self, const ReplicatorConfiguration* configuration)
{
	self->setConfiguration(configuration ? configuration->getConfiguration() : net::Replicator::Configuration());
}

Ref< ReplicatorConfiguration > net_Replicator_getConfiguration(net::Replicator* self)
{
	return new ReplicatorConfiguration(self->getConfiguration());
}

		}

void registerNetClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< net::State > > classState = new script::AutoScriptClass< net::State >();
	scriptManager->registerClass(classState);

	Ref< script::AutoScriptClass< net::StateTemplate > > classStateTemplate = new script::AutoScriptClass< net::StateTemplate >();
	classStateTemplate->addMethod("extrapolate", &net::StateTemplate::extrapolate);
	scriptManager->registerClass(classStateTemplate);

	Ref< script::AutoScriptClass< net::INetworkTopology > > classINetworkTopology = new script::AutoScriptClass< net::INetworkTopology >();
	scriptManager->registerClass(classINetworkTopology);

	Ref< script::AutoScriptClass< net::IPeer2PeerProvider > > classIPeer2PeerProvider = new script::AutoScriptClass< net::IPeer2PeerProvider >();
	scriptManager->registerClass(classIPeer2PeerProvider);

	Ref< script::AutoScriptClass< net::Peer2PeerTopology > > classPeer2PeerTopology = new script::AutoScriptClass< net::Peer2PeerTopology >();
	classPeer2PeerTopology->addConstructor< net::IPeer2PeerProvider* >();
	scriptManager->registerClass(classPeer2PeerTopology);

	Ref< script::AutoScriptClass< net::ReplicatorProxy > > classReplicatorProxy = new script::AutoScriptClass< net::ReplicatorProxy >();
	classReplicatorProxy->addMethod("getHandle", &net::ReplicatorProxy::getHandle);
	classReplicatorProxy->addMethod("getName", &net::ReplicatorProxy::getName);
	classReplicatorProxy->addMethod("getStatus", &net::ReplicatorProxy::getStatus);
	classReplicatorProxy->addMethod("getLatency", &net::ReplicatorProxy::getLatency);
	classReplicatorProxy->addMethod("getLatencySpread", &net::ReplicatorProxy::getLatencySpread);
	classReplicatorProxy->addMethod("getReverseLatency", &net::ReplicatorProxy::getReverseLatency);
	classReplicatorProxy->addMethod("getReverseLatencySpread", &net::ReplicatorProxy::getReverseLatencySpread);
	classReplicatorProxy->addMethod("isConnected", &net::ReplicatorProxy::isConnected);
	classReplicatorProxy->addMethod("setPrimary", &net::ReplicatorProxy::setPrimary);
	classReplicatorProxy->addMethod("isPrimary", &net::ReplicatorProxy::isPrimary);
	classReplicatorProxy->addMethod("isRelayed", &net::ReplicatorProxy::isRelayed);
	classReplicatorProxy->addMethod("setObject", &net::ReplicatorProxy::setObject);
	classReplicatorProxy->addMethod("getObject", &net::ReplicatorProxy::getObject);
	classReplicatorProxy->addMethod("setOrigin", &net::ReplicatorProxy::setOrigin);
	classReplicatorProxy->addMethod("getOrigin", &net::ReplicatorProxy::getOrigin);
	classReplicatorProxy->addMethod("setStateTemplate", &net::ReplicatorProxy::setStateTemplate);
	classReplicatorProxy->addMethod("getStateTemplate", &net::ReplicatorProxy::getStateTemplate);
	classReplicatorProxy->addMethod("getState", &net::ReplicatorProxy::getState);
	classReplicatorProxy->addMethod("resetStates", &net::ReplicatorProxy::resetStates);
	classReplicatorProxy->addMethod("setSendState", &net::ReplicatorProxy::setSendState);
	classReplicatorProxy->addMethod("sendEvent", &net::ReplicatorProxy::sendEvent);
	scriptManager->registerClass(classReplicatorProxy);

	Ref< script::AutoScriptClass< net::Replicator::IListener > > classReplicatorIListener = new script::AutoScriptClass< net::Replicator::IListener >();
	scriptManager->registerClass(classReplicatorIListener);

	Ref< script::AutoScriptClass< net::Replicator::IEventListener > > classReplicatorIEventListener = new script::AutoScriptClass< net::Replicator::IEventListener >();
	scriptManager->registerClass(classReplicatorIEventListener);

	Ref< script::AutoScriptClass< ReplicatorConfiguration > > classReplicatorConfiguration = new script::AutoScriptClass< ReplicatorConfiguration >();
	classReplicatorConfiguration->addConstructor();
	classReplicatorConfiguration->addMethod("setNearDistance", &ReplicatorConfiguration::setNearDistance);
	classReplicatorConfiguration->addMethod("getNearDistance", &ReplicatorConfiguration::getNearDistance);
	classReplicatorConfiguration->addMethod("setFarDistance", &ReplicatorConfiguration::setFarDistance);
	classReplicatorConfiguration->addMethod("getFarDistance", &ReplicatorConfiguration::getFarDistance);
	classReplicatorConfiguration->addMethod("setFurthestDistance", &ReplicatorConfiguration::setFurthestDistance);
	classReplicatorConfiguration->addMethod("getFurthestDistance", &ReplicatorConfiguration::getFurthestDistance);
	classReplicatorConfiguration->addMethod("setTimeUntilTxStateNear", &ReplicatorConfiguration::setTimeUntilTxStateNear);
	classReplicatorConfiguration->addMethod("getTimeUntilTxStateNear", &ReplicatorConfiguration::getTimeUntilTxStateNear);
	classReplicatorConfiguration->addMethod("setTimeUntilTxStateFar", &ReplicatorConfiguration::setTimeUntilTxStateFar);
	classReplicatorConfiguration->addMethod("getTimeUntilTxStateFar", &ReplicatorConfiguration::getTimeUntilTxStateFar);
	classReplicatorConfiguration->addMethod("setTimeUntilTxPing", &ReplicatorConfiguration::setTimeUntilTxPing);
	classReplicatorConfiguration->addMethod("getTimeUntilTxPing", &ReplicatorConfiguration::getTimeUntilTxPing);
	scriptManager->registerClass(classReplicatorConfiguration);

	Ref< script::AutoScriptClass< ReplicatorListener > > classReplicatorListener = new script::AutoScriptClass< ReplicatorListener >();
	classReplicatorListener->addConstructor< script::IScriptDelegate* >();
	scriptManager->registerClass(classReplicatorListener);

	Ref< script::AutoScriptClass< ReplicatorEventListener > > classReplicatorEventListener = new script::AutoScriptClass< ReplicatorEventListener >();
	classReplicatorEventListener->addConstructor< script::IScriptDelegate* >();
	scriptManager->registerClass(classReplicatorEventListener);

	Ref< script::AutoScriptClass< net::Replicator > > classReplicator = new script::AutoScriptClass< net::Replicator >();
	classReplicator->addConstructor();
	classReplicator->addMethod("create", &net_Replicator_create);
	classReplicator->addMethod("destroy", &net::Replicator::destroy);
	classReplicator->addMethod("setConfiguration", &net_Replicator_setConfiguration);
	classReplicator->addMethod("getConfiguration", &net_Replicator_getConfiguration);
	classReplicator->addMethod("addEventType", &net::Replicator::addEventType);
	classReplicator->addMethod("removeAllEventTypes", &net::Replicator::removeAllEventTypes);
	classReplicator->addMethod("addListener", &net::Replicator::addListener);
	classReplicator->addMethod("removeListener", &net::Replicator::removeListener);
	classReplicator->addMethod("removeAllListeners", &net::Replicator::removeAllListeners);
	classReplicator->addMethod("addEventListener", &net::Replicator::addEventListener);
	classReplicator->addMethod("removeEventListener", &net::Replicator::removeEventListener);
	classReplicator->addMethod("removeAllEventListeners", &net::Replicator::removeAllEventListeners);
	classReplicator->addMethod("update", &net::Replicator::update);
	classReplicator->addMethod("getName", &net::Replicator::getName);
	classReplicator->addMethod("setStatus", &net::Replicator::setStatus);
	classReplicator->addMethod("getStatus", &net::Replicator::getStatus);
	classReplicator->addMethod("isPrimary", &net::Replicator::isPrimary);
	classReplicator->addMethod("setOrigin", &net::Replicator::setOrigin);
	classReplicator->addMethod("setStateTemplate", &net::Replicator::setStateTemplate);
	classReplicator->addMethod("setState", &net::Replicator::setState);
	classReplicator->addMethod("getState", &net::Replicator::getState);
	classReplicator->addMethod("setSendState", &net::Replicator::setSendState);
	classReplicator->addMethod("getProxyCount", &net::Replicator::getProxyCount);
	classReplicator->addMethod("getProxy", &net::Replicator::getProxy);
	classReplicator->addMethod("getPrimaryProxy", &net::Replicator::getPrimaryProxy);
	classReplicator->addMethod("getAverageLatency", &net::Replicator::getAverageLatency);
	classReplicator->addMethod("broadcastEvent", &net::Replicator::broadcastEvent);
	classReplicator->addMethod("sendEventToPrimary", &net::Replicator::sendEventToPrimary);
	classReplicator->addMethod("getTime", &net::Replicator::getTime);
	classReplicator->addMethod("getTimeVariance", &net::Replicator::getTimeVariance);
	classReplicator->addMethod("isTimeSynchronized", &net::Replicator::isTimeSynchronized);
	scriptManager->registerClass(classReplicator);
}

	}
}