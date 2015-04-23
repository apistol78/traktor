#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Class/IRuntimeDelegate.h"
#include "Core/Io/IStream.h"
#include "Net/NetClassFactory.h"
#include "Net/Replication/INetworkTopology.h"
#include "Net/Replication/IPeer2PeerProvider.h"
#include "Net/Replication/IReplicatorEventListener.h"
#include "Net/Replication/IReplicatorStateListener.h"
#include "Net/Replication/MeasureP2PProvider.h"
#include "Net/Replication/Peer2PeerTopology.h"
#include "Net/Replication/Replicator.h"
#include "Net/Replication/ReplicatorProxy.h"
#include "Net/Replication/State/State.h"
#include "Net/Replication/State/StateTemplate.h"

namespace traktor
{
	namespace net
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

	ReplicatorConfiguration(const Replicator::Configuration& configuration)
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

	const Replicator::Configuration& getConfiguration() const { return m_configuration; }

private:
	Replicator::Configuration m_configuration;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.ReplicatorConfiguration", ReplicatorConfiguration, Object)

class ReplicatorListener : public IReplicatorStateListener
{
	T_RTTI_CLASS;

public:
	ReplicatorListener(IRuntimeDelegate* delegate)
	:	m_delegate(delegate)
	{
	}

	virtual void notify(Replicator* replicator, float eventTime, uint32_t eventId, ReplicatorProxy* proxy, const Object* eventObject)
	{
		Any argv[] =
		{
			CastAny< Object* >::set(replicator),
			CastAny< float >::set(eventTime),
			CastAny< int32_t >::set(int32_t(eventId)),
			CastAny< Object* >::set(proxy),
			CastAny< Object* >::set((Object*)eventObject)
		};
		if (m_delegate)
			m_delegate->call(sizeof_array(argv), argv);
	}

private:
	Ref< IRuntimeDelegate > m_delegate;
};

class ReplicatorEventListener : public IReplicatorEventListener
{
	T_RTTI_CLASS;

public:
	ReplicatorEventListener(IRuntimeDelegate* delegate)
	:	m_delegate(delegate)
	{
	}

	virtual void notify(Replicator* replicator, float eventTime, ReplicatorProxy* fromProxy, const Object* eventObject)
	{
		Any argv[] =
		{
			CastAny< Object* >::set(replicator),
			CastAny< float >::set(eventTime),
			CastAny< Object* >::set(fromProxy),
			CastAny< Object* >::set((Object*)eventObject)
		};
		if (m_delegate)
			m_delegate->call(sizeof_array(argv), argv);
	}

private:
	Ref< IRuntimeDelegate > m_delegate;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.ReplicatorListener", ReplicatorListener, IReplicatorStateListener)

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.ReplicatorEventListener", ReplicatorEventListener, IReplicatorEventListener)

bool net_Replicator_create(Replicator* self, INetworkTopology* topology, const ReplicatorConfiguration* configuration)
{
	return self->create(topology, configuration ? configuration->getConfiguration() : Replicator::Configuration());
}

void net_Replicator_setConfiguration(Replicator* self, const ReplicatorConfiguration* configuration)
{
	self->setConfiguration(configuration ? configuration->getConfiguration() : Replicator::Configuration());
}

Ref< ReplicatorConfiguration > net_Replicator_getConfiguration(Replicator* self)
{
	return new ReplicatorConfiguration(self->getConfiguration());
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.net.NetClassFactory", 0, NetClassFactory, IRuntimeClassFactory)

void NetClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< State > > classState = new AutoRuntimeClass< State >();
	registrar->registerClass(classState);

	Ref< AutoRuntimeClass< StateTemplate > > classStateTemplate = new AutoRuntimeClass< StateTemplate >();
	classStateTemplate->addMethod("extrapolate", &StateTemplate::extrapolate);
	registrar->registerClass(classStateTemplate);

	Ref< AutoRuntimeClass< INetworkTopology > > classINetworkTopology = new AutoRuntimeClass< INetworkTopology >();
	registrar->registerClass(classINetworkTopology);

	Ref< AutoRuntimeClass< IPeer2PeerProvider > > classIPeer2PeerProvider = new AutoRuntimeClass< IPeer2PeerProvider >();
	registrar->registerClass(classIPeer2PeerProvider);

	Ref< AutoRuntimeClass< MeasureP2PProvider > > classMeasureP2PProvider = new AutoRuntimeClass< MeasureP2PProvider >();
	classMeasureP2PProvider->addConstructor< IPeer2PeerProvider* >();
	classMeasureP2PProvider->addMethod("getSendBitsPerSecond", &MeasureP2PProvider::getSendBitsPerSecond);
	classMeasureP2PProvider->addMethod("getRecvBitsPerSecond", &MeasureP2PProvider::getRecvBitsPerSecond);
	registrar->registerClass(classMeasureP2PProvider);

	Ref< AutoRuntimeClass< Peer2PeerTopology > > classPeer2PeerTopology = new AutoRuntimeClass< Peer2PeerTopology >();
	classPeer2PeerTopology->addConstructor< IPeer2PeerProvider* >();
	registrar->registerClass(classPeer2PeerTopology);

	Ref< AutoRuntimeClass< ReplicatorProxy > > classReplicatorProxy = new AutoRuntimeClass< ReplicatorProxy >();
	classReplicatorProxy->addMethod("getHandle", &ReplicatorProxy::getHandle);
	classReplicatorProxy->addMethod("getName", &ReplicatorProxy::getName);
	classReplicatorProxy->addMethod("getUser", &ReplicatorProxy::getUser);
	classReplicatorProxy->addMethod("getStatus", &ReplicatorProxy::getStatus);
	classReplicatorProxy->addMethod("isLatencyReliable", &ReplicatorProxy::isLatencyReliable);
	classReplicatorProxy->addMethod("getLatency", &ReplicatorProxy::getLatency);
	classReplicatorProxy->addMethod("getLatencySpread", &ReplicatorProxy::getLatencySpread);
	classReplicatorProxy->addMethod("getReverseLatency", &ReplicatorProxy::getReverseLatency);
	classReplicatorProxy->addMethod("getReverseLatencySpread", &ReplicatorProxy::getReverseLatencySpread);
	classReplicatorProxy->addMethod("resetLatencies", &ReplicatorProxy::resetLatencies);
	classReplicatorProxy->addMethod("getTimeRate", &ReplicatorProxy::getTimeRate);
	classReplicatorProxy->addMethod("isConnected", &ReplicatorProxy::isConnected);
	classReplicatorProxy->addMethod("setPrimary", &ReplicatorProxy::setPrimary);
	classReplicatorProxy->addMethod("isPrimary", &ReplicatorProxy::isPrimary);
	classReplicatorProxy->addMethod("isRelayed", &ReplicatorProxy::isRelayed);
	classReplicatorProxy->addMethod("setObject", &ReplicatorProxy::setObject);
	classReplicatorProxy->addMethod("getObject", &ReplicatorProxy::getObject);
	classReplicatorProxy->addMethod("setOrigin", &ReplicatorProxy::setOrigin);
	classReplicatorProxy->addMethod("getOrigin", &ReplicatorProxy::getOrigin);
	classReplicatorProxy->addMethod("setStateTemplate", &ReplicatorProxy::setStateTemplate);
	classReplicatorProxy->addMethod("getStateTemplate", &ReplicatorProxy::getStateTemplate);
	classReplicatorProxy->addMethod("getState", &ReplicatorProxy::getState);
	classReplicatorProxy->addMethod("getFilteredState", &ReplicatorProxy::getFilteredState);
	classReplicatorProxy->addMethod("resetStates", &ReplicatorProxy::resetStates);
	classReplicatorProxy->addMethod("setSendState", &ReplicatorProxy::setSendState);
	classReplicatorProxy->addMethod("sendEvent", &ReplicatorProxy::sendEvent);
	registrar->registerClass(classReplicatorProxy);

	Ref< AutoRuntimeClass< IReplicatorStateListener > > classIReplicatorStateListener = new AutoRuntimeClass< IReplicatorStateListener >();
	registrar->registerClass(classIReplicatorStateListener);

	Ref< AutoRuntimeClass< IReplicatorEventListener > > classIReplicatorEventListener = new AutoRuntimeClass< IReplicatorEventListener >();
	registrar->registerClass(classIReplicatorEventListener);

	Ref< AutoRuntimeClass< ReplicatorConfiguration > > classReplicatorConfiguration = new AutoRuntimeClass< ReplicatorConfiguration >();
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
	registrar->registerClass(classReplicatorConfiguration);

	Ref< AutoRuntimeClass< ReplicatorListener > > classReplicatorListener = new AutoRuntimeClass< ReplicatorListener >();
	classReplicatorListener->addConstructor< IRuntimeDelegate* >();
	registrar->registerClass(classReplicatorListener);

	Ref< AutoRuntimeClass< ReplicatorEventListener > > classReplicatorEventListener = new AutoRuntimeClass< ReplicatorEventListener >();
	classReplicatorEventListener->addConstructor< IRuntimeDelegate* >();
	registrar->registerClass(classReplicatorEventListener);

	Ref< AutoRuntimeClass< Replicator > > classReplicator = new AutoRuntimeClass< Replicator >();
	classReplicator->addConstructor();
	classReplicator->addMethod("create", &net_Replicator_create);
	classReplicator->addMethod("destroy", &Replicator::destroy);
	classReplicator->addMethod("setConfiguration", &net_Replicator_setConfiguration);
	classReplicator->addMethod("getConfiguration", &net_Replicator_getConfiguration);
	classReplicator->addMethod("addEventType", &Replicator::addEventType);
	classReplicator->addMethod("removeAllEventTypes", &Replicator::removeAllEventTypes);
	classReplicator->addMethod("addListener", &Replicator::addListener);
	classReplicator->addMethod("removeListener", &Replicator::removeListener);
	classReplicator->addMethod("removeAllListeners", &Replicator::removeAllListeners);
	classReplicator->addMethod("addEventListener", &Replicator::addEventListener);
	classReplicator->addMethod("removeEventListener", &Replicator::removeEventListener);
	classReplicator->addMethod("removeAllEventListeners", &Replicator::removeAllEventListeners);
	classReplicator->addMethod("update", &Replicator::update);
	classReplicator->addMethod("flush", &Replicator::flush);
	classReplicator->addMethod("getName", &Replicator::getName);
	classReplicator->addMethod("setStatus", &Replicator::setStatus);
	classReplicator->addMethod("getStatus", &Replicator::getStatus);
	classReplicator->addMethod("isPrimary", &Replicator::isPrimary);
	classReplicator->addMethod("setOrigin", &Replicator::setOrigin);
	classReplicator->addMethod("setStateTemplate", &Replicator::setStateTemplate);
	classReplicator->addMethod("setState", &Replicator::setState);
	classReplicator->addMethod("getState", &Replicator::getState);
	classReplicator->addMethod("setSendState", &Replicator::setSendState);
	classReplicator->addMethod("getProxyCount", &Replicator::getProxyCount);
	classReplicator->addMethod("getProxy", &Replicator::getProxy);
	classReplicator->addMethod("getPrimaryProxy", &Replicator::getPrimaryProxy);
	classReplicator->addMethod("resetAllLatencies", &Replicator::resetAllLatencies);
	classReplicator->addMethod("getAverageLatency", &Replicator::getAverageLatency);
	classReplicator->addMethod("getAverageReverseLatency", &Replicator::getAverageReverseLatency);
	classReplicator->addMethod("getBestLatency", &Replicator::getBestLatency);
	classReplicator->addMethod("getBestReverseLatency", &Replicator::getBestReverseLatency);
	classReplicator->addMethod("getWorstLatency", &Replicator::getWorstLatency);
	classReplicator->addMethod("getWorstReverseLatency", &Replicator::getWorstReverseLatency);
	classReplicator->addMethod("broadcastEvent", &Replicator::broadcastEvent);
	classReplicator->addMethod("sendEventToPrimary", &Replicator::sendEventToPrimary);
	classReplicator->addMethod("getTime", &Replicator::getTime);
	classReplicator->addMethod("getTimeVariance", &Replicator::getTimeVariance);
	classReplicator->addMethod("isTimeSynchronized", &Replicator::isTimeSynchronized);
	registrar->registerClass(classReplicator);
}

	}
}
