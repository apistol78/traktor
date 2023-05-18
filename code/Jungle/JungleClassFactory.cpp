/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes/BoxedTransform.h"
#include "Core/Class/Boxes/BoxedTypeInfo.h"
#include "Core/Class/Boxes/BoxedVector4.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Class/IRuntimeDelegate.h"
#include "Core/Io/IStream.h"
#include "Core/Io/OutputStream.h"
#include "Core/Serialization/ISerializable.h"
#include "Jungle/INetworkTopology.h"
#include "Jungle/IPeer2PeerProvider.h"
#include "Jungle/IReplicatorEventListener.h"
#include "Jungle/IReplicatorStateListener.h"
#include "Jungle/JungleClassFactory.h"
#include "Jungle/MeasureP2PProvider.h"
#include "Jungle/OnlinePeer2PeerProvider.h"
#include "Jungle/Peer2PeerTopology.h"
#include "Jungle/Replicator.h"
#include "Jungle/ReplicatorProxy.h"
#include "Jungle/State/BodyStateTemplate.h"
#include "Jungle/State/BodyStateValue.h"
#include "Jungle/State/BooleanTemplate.h"
#include "Jungle/State/BooleanValue.h"
#include "Jungle/State/FloatTemplate.h"
#include "Jungle/State/FloatValue.h"
#include "Jungle/State/State.h"
#include "Jungle/State/StateTemplate.h"
#include "Jungle/State/TransformTemplate.h"
#include "Jungle/State/TransformValue.h"
#include "Jungle/State/VectorTemplate.h"
#include "Jungle/State/VectorValue.h"
#include "Online/ILobby.h"
#include "Online/ISessionManager.h"
#include "Physics/BoxedBodyState.h"

namespace traktor::jungle
{
	namespace
	{

class ReplicatorConfiguration : public Object
{
	T_RTTI_CLASS;

public:
	ReplicatorConfiguration() = default;

	ReplicatorConfiguration(const Replicator::Configuration& configuration)
	:	m_configuration(configuration)
	{
	}

	void setTimeVarianceThreshold(float timeVarianceThreshold) { m_configuration.timeVarianceThreshold = timeVarianceThreshold; }

	float getTimeVarianceThreshold() const { return m_configuration.timeVarianceThreshold; }

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.jungle.ReplicatorConfiguration", ReplicatorConfiguration, Object)

class ReplicatorListener : public IReplicatorStateListener
{
	T_RTTI_CLASS;

public:
	ReplicatorListener(IRuntimeDelegate* delegate)
	:	m_delegate(delegate)
	{
	}

	virtual void notify(Replicator* replicator, float eventTime, uint32_t eventId, ReplicatorProxy* proxy, const Object* eventObject) final
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

	virtual bool notify(Replicator* replicator, float eventTime, ReplicatorProxy* fromProxy, const Object* eventObject) final
	{
		Any argv[] =
		{
			CastAny< Object* >::set(replicator),
			CastAny< float >::set(eventTime),
			CastAny< Object* >::set(fromProxy),
			CastAny< Object* >::set((Object*)eventObject)
		};
		if (m_delegate)
			return m_delegate->call(sizeof_array(argv), argv).getBoolean();
		else
			return false;
	}

private:
	Ref< IRuntimeDelegate > m_delegate;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.jungle.ReplicatorListener", ReplicatorListener, IReplicatorStateListener)

T_IMPLEMENT_RTTI_CLASS(L"traktor.jungle.ReplicatorEventListener", ReplicatorEventListener, IReplicatorEventListener)

void State_pack(State* self, const IValue* value)
{
	self->pack(value);
}

const IValue* State_unpack(State* self)
{
	return self->unpack();
}

bool Replicator_create(Replicator* self, INetworkTopology* topology, const ReplicatorConfiguration* configuration)
{
	return self->create(topology, configuration ? configuration->getConfiguration() : Replicator::Configuration());
}

void Replicator_setConfiguration(Replicator* self, const ReplicatorConfiguration* configuration)
{
	self->setConfiguration(configuration ? configuration->getConfiguration() : Replicator::Configuration());
}

Ref< ReplicatorConfiguration > Replicator_getConfiguration(Replicator* self)
{
	return new ReplicatorConfiguration(self->getConfiguration());
}

Ref< Object > Replicator_addListener(Replicator* self, IRuntimeDelegate* delegate)
{
	Ref< IReplicatorStateListener > listener = new ReplicatorListener(delegate);
	self->addListener(listener);
	return listener;
}

void Replicator_removeListener(Replicator* self, Object* listener)
{
	self->removeListener(mandatory_non_null_type_cast< IReplicatorStateListener* >(listener));
}

Ref< Object > Replicator_addEventListener(Replicator* self, const TypeInfo& eventType, IRuntimeDelegate* delegate)
{
	Ref< IReplicatorEventListener > listener = new ReplicatorEventListener(delegate);
	self->addEventListener(eventType, listener);
	return listener;
}

void Replicator_removeEventListener(Replicator* self, Object* listener)
{
	self->removeEventListener(mandatory_non_null_type_cast< IReplicatorEventListener* >(listener));
}

physics::BodyState BodyStateValue_get(BodyStateValue* self)
{
	return *self;
}

bool BooleanValue_get(BooleanValue* self)
{
	return *self;
}

float FloatValue_get(FloatValue* self)
{
	return *self;
}

Transform TransformValue_get(TransformValue* self)
{
	return *self;
}

Vector4 VectorValue_get(VectorValue* self)
{
	return *self;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.jungle.JungleClassFactory", 0, JungleClassFactory, IRuntimeClassFactory)

void JungleClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classBodyStateValue = new AutoRuntimeClass< BodyStateValue >();
	classBodyStateValue->addConstructor< const physics::BodyState& >();
	classBodyStateValue->addMethod("get", &BodyStateValue_get);
	registrar->registerClass(classBodyStateValue);

	auto classBodyStateTemplate = new AutoRuntimeClass< BodyStateTemplate >();
	classBodyStateTemplate->addConstructor < const std::wstring& >();
	registrar->registerClass(classBodyStateTemplate);

	auto classBooleanValue = new AutoRuntimeClass< BooleanValue >();
	classBooleanValue->addConstructor< bool >();
	classBooleanValue->addMethod("get", &BooleanValue_get);
	registrar->registerClass(classBooleanValue);

	auto classBooleanTemplate = new AutoRuntimeClass< BooleanTemplate >();
	classBooleanTemplate->addConstructor< const std::wstring&, float >();
	registrar->registerClass(classBooleanTemplate);

	auto classFloatValue = new AutoRuntimeClass< FloatValue >();
	classFloatValue->addConstructor< float >();
	classFloatValue->addMethod("get", &FloatValue_get);
	registrar->registerClass(classFloatValue);

	auto classFloatTemplate = new AutoRuntimeClass< FloatTemplate >();
	classFloatTemplate->addConstructor< const std::wstring& >();
	classFloatTemplate->addConstructor< const std::wstring&, float >();
	//classFloatTemplate->addConstructor< const std::wstring&, float, float, float, FloatTemplatePrecision, bool >();
	registrar->registerClass(classFloatTemplate);

	auto classTransformValue = new AutoRuntimeClass< TransformValue >();
	classTransformValue->addConstructor< const Transform& >();
	classTransformValue->addMethod("get", &TransformValue_get);
	registrar->registerClass(classTransformValue);

	auto classTransformTemplate = new AutoRuntimeClass< TransformTemplate >();
	classTransformTemplate->addConstructor< const std::wstring& >();
	registrar->registerClass(classTransformTemplate);

	auto classVectorValue = new AutoRuntimeClass< VectorValue >();
	classVectorValue->addConstructor< const Vector4& >();
	classVectorValue->addMethod("get", &VectorValue_get);
	registrar->registerClass(classVectorValue);

	auto classVectorTemplate = new AutoRuntimeClass< VectorTemplate >();
	classVectorTemplate->addConstructor< const std::wstring& >();
	registrar->registerClass(classVectorTemplate);

	auto classIValue = new AutoRuntimeClass< IValue >();
	registrar->registerClass(classIValue);

	auto classIValueTemplate = new AutoRuntimeClass< IValueTemplate >();
	registrar->registerClass(classIValueTemplate);

	auto classState = new AutoRuntimeClass< State >();
	classState->addConstructor();
	classState->addMethod("packBegin", &State::packBegin);
	classState->addMethod("pack", &State_pack);
	classState->addMethod("unpackBegin", &State::unpackBegin);
	classState->addMethod("unpack", &State_unpack);
	registrar->registerClass(classState);

	auto classStateTemplate = new AutoRuntimeClass< StateTemplate >();
	classStateTemplate->addConstructor();
	classStateTemplate->addMethod("declare", &StateTemplate::declare);
	classStateTemplate->addMethod("match", &StateTemplate::match);
	classStateTemplate->addMethod("critical", &StateTemplate::critical);
	classStateTemplate->addMethod("extrapolate", &StateTemplate::extrapolate);
	registrar->registerClass(classStateTemplate);

	auto classINetworkTopology = new AutoRuntimeClass< INetworkTopology >();
	registrar->registerClass(classINetworkTopology);

	auto classIPeer2PeerProvider = new AutoRuntimeClass< IPeer2PeerProvider >();
	registrar->registerClass(classIPeer2PeerProvider);

	auto classOnlinePeer2PeerProvider = new AutoRuntimeClass< OnlinePeer2PeerProvider >();
	classOnlinePeer2PeerProvider->addConstructor< online::ISessionManager*, online::ILobby*, bool, bool >();
	registrar->registerClass(classOnlinePeer2PeerProvider);

	auto classMeasureP2PProvider = new AutoRuntimeClass< MeasureP2PProvider >();
	classMeasureP2PProvider->addConstructor< IPeer2PeerProvider* >();
	classMeasureP2PProvider->addProperty("sendBitsPerSecond", &MeasureP2PProvider::getSendBitsPerSecond);
	classMeasureP2PProvider->addProperty("recvBitsPerSecond", &MeasureP2PProvider::getRecvBitsPerSecond);
	registrar->registerClass(classMeasureP2PProvider);

	auto classPeer2PeerTopology = new AutoRuntimeClass< Peer2PeerTopology >();
	classPeer2PeerTopology->addConstructor< IPeer2PeerProvider* >();
	classPeer2PeerTopology->addMethod("setIAmInterval", &Peer2PeerTopology::setIAmInterval);
	classPeer2PeerTopology->addMethod("setPropagateCMaskInterval", &Peer2PeerTopology::setPropagateCMaskInterval);
	registrar->registerClass(classPeer2PeerTopology);

	auto classReplicatorProxy = new AutoRuntimeClass< ReplicatorProxy >();
	classReplicatorProxy->addProperty("handle", &ReplicatorProxy::getHandle);
	classReplicatorProxy->addProperty("name", &ReplicatorProxy::getName);
	classReplicatorProxy->addProperty("user", &ReplicatorProxy::getUser);
	classReplicatorProxy->addProperty("status", &ReplicatorProxy::getStatus);
	classReplicatorProxy->addProperty("latencyReliable", &ReplicatorProxy::isLatencyReliable);
	classReplicatorProxy->addProperty("latency", &ReplicatorProxy::getLatency);
	classReplicatorProxy->addProperty("latencySpread", &ReplicatorProxy::getLatencySpread);
	classReplicatorProxy->addProperty("reverseLatency", &ReplicatorProxy::getReverseLatency);
	classReplicatorProxy->addProperty("reverseLatencySpread", &ReplicatorProxy::getReverseLatencySpread);
	classReplicatorProxy->addProperty("timeRate", &ReplicatorProxy::getTimeRate);
	classReplicatorProxy->addProperty("connected", &ReplicatorProxy::isConnected);
	classReplicatorProxy->addProperty("primary", &ReplicatorProxy::isPrimary);
	classReplicatorProxy->addProperty("relayed", &ReplicatorProxy::isRelayed);
	classReplicatorProxy->addProperty("object", &ReplicatorProxy::setObject, &ReplicatorProxy::getObject);
	classReplicatorProxy->addProperty("origin", &ReplicatorProxy::setOrigin, &ReplicatorProxy::getOrigin);
	classReplicatorProxy->addProperty("stateTemplate", &ReplicatorProxy::setStateTemplate, &ReplicatorProxy::getStateTemplate);
	classReplicatorProxy->addMethod("getState", &ReplicatorProxy::getState);
	classReplicatorProxy->addMethod("getFilteredState", &ReplicatorProxy::getFilteredState);
	classReplicatorProxy->addMethod("setPrimary", &ReplicatorProxy::setPrimary);
	classReplicatorProxy->addMethod("resetLatencies", &ReplicatorProxy::resetLatencies);
	classReplicatorProxy->addMethod("resetStates", &ReplicatorProxy::resetStates);
	classReplicatorProxy->addMethod("setSendState", &ReplicatorProxy::setSendState);
	classReplicatorProxy->addMethod("sendEvent", &ReplicatorProxy::sendEvent);
	registrar->registerClass(classReplicatorProxy);

	auto classIReplicatorStateListener = new AutoRuntimeClass< IReplicatorStateListener >();
	registrar->registerClass(classIReplicatorStateListener);

	auto classIReplicatorEventListener = new AutoRuntimeClass< IReplicatorEventListener >();
	registrar->registerClass(classIReplicatorEventListener);

	auto classReplicatorConfiguration = new AutoRuntimeClass< ReplicatorConfiguration >();
	classReplicatorConfiguration->addConstructor();
	classReplicatorConfiguration->addProperty("timeVarianceThreshold", &ReplicatorConfiguration::setTimeVarianceThreshold, &ReplicatorConfiguration::getTimeVarianceThreshold);
	classReplicatorConfiguration->addProperty("nearDistance", &ReplicatorConfiguration::setNearDistance, &ReplicatorConfiguration::getNearDistance);
	classReplicatorConfiguration->addProperty("farDistance", &ReplicatorConfiguration::setFarDistance, &ReplicatorConfiguration::getFarDistance);
	classReplicatorConfiguration->addProperty("furthestDistance", &ReplicatorConfiguration::setFurthestDistance, &ReplicatorConfiguration::getFurthestDistance);
	classReplicatorConfiguration->addProperty("timeUntilTxStateNear", &ReplicatorConfiguration::setTimeUntilTxStateNear, &ReplicatorConfiguration::getTimeUntilTxStateNear);
	classReplicatorConfiguration->addProperty("timeUntilTxStateFar", &ReplicatorConfiguration::setTimeUntilTxStateFar, &ReplicatorConfiguration::getTimeUntilTxStateFar);
	classReplicatorConfiguration->addProperty("timeUntilTxPing", &ReplicatorConfiguration::setTimeUntilTxPing, &ReplicatorConfiguration::getTimeUntilTxPing);
	registrar->registerClass(classReplicatorConfiguration);

	auto classReplicator = new AutoRuntimeClass< Replicator >();
	classReplicator->addConstructor();
	classReplicator->addProperty("name", &Replicator::getName);
	classReplicator->addProperty("status", &Replicator::setStatus, &Replicator::getStatus);
	classReplicator->addProperty("primary", &Replicator::isPrimary);
	classReplicator->addProperty("state", &Replicator::setState, &Replicator::getState);
	classReplicator->addProperty("averageLatency", &Replicator::getAverageLatency);
	classReplicator->addProperty("averageReverseLatency", &Replicator::getAverageReverseLatency);
	classReplicator->addProperty("bestLatency", &Replicator::getBestLatency);
	classReplicator->addProperty("bestReverseLatency", &Replicator::getBestReverseLatency);
	classReplicator->addProperty("worstLatency", &Replicator::getWorstLatency);
	classReplicator->addProperty("worstReverseLatency", &Replicator::getWorstReverseLatency);
	classReplicator->addProperty("time", &Replicator::getTime);
	classReplicator->addProperty("timeVariance", &Replicator::getTimeVariance);
	classReplicator->addProperty("timeSynchronized", &Replicator::isTimeSynchronized);
	classReplicator->addProperty("proxyCount", &Replicator::getProxyCount);
	classReplicator->addProperty("primaryProxy", &Replicator::getPrimaryProxy);
	classReplicator->addMethod("create", &Replicator_create);
	classReplicator->addMethod("destroy", &Replicator::destroy);
	classReplicator->addMethod("setConfiguration", &Replicator_setConfiguration);
	classReplicator->addMethod("getConfiguration", &Replicator_getConfiguration);
	classReplicator->addMethod("addEventType", &Replicator::addEventType);
	classReplicator->addMethod("removeAllEventTypes", &Replicator::removeAllEventTypes);
	classReplicator->addMethod("addListener", &Replicator_addListener);
	classReplicator->addMethod("removeListener", &Replicator_removeListener);
	classReplicator->addMethod("removeAllListeners", &Replicator::removeAllListeners);
	classReplicator->addMethod("addEventListener", &Replicator_addEventListener);
	classReplicator->addMethod("removeEventListener", &Replicator_removeEventListener);
	classReplicator->addMethod("removeAllEventListeners", &Replicator::removeAllEventListeners);
	classReplicator->addMethod("update", &Replicator::update);
	classReplicator->addMethod("flush", &Replicator::flush);
	classReplicator->addMethod("setOrigin", &Replicator::setOrigin);
	classReplicator->addMethod("setStateTemplate", &Replicator::setStateTemplate);
	classReplicator->addMethod("setSendState", &Replicator::setSendState);
	classReplicator->addMethod("getProxy", &Replicator::getProxy);
	classReplicator->addMethod("resetAllLatencies", &Replicator::resetAllLatencies);
	classReplicator->addMethod("broadcastEvent", &Replicator::broadcastEvent);
	classReplicator->addMethod("sendEventToPrimary", &Replicator::sendEventToPrimary);
	classReplicator->addMethod("setTimeSynchronization", &Replicator::setTimeSynchronization);
	registrar->registerClass(classReplicator);
}

}
