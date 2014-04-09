#include "Core/Io/IStream.h"
#include "Online/ILobby.h"
#include "Online/ISessionManager.h"
#include "Net/Replication/BandwidthLimitPeers.h"
#include "Net/Replication/DiagnosePeers.h"
#include "Net/Replication/InetSimPeers.h"
#include "Net/Replication/RecordPeers.h"
#include "Net/Replication/RelayPeers.h"
#include "Net/Replication/ReliableTransportPeers.h"
#include "Net/Replication/Replicator.h"
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

	void setNearTimeUntilTx(int32_t nearTimeUntilTx) { m_configuration.nearTimeUntilTx = nearTimeUntilTx; }

	int32_t getNearTimeUntilTx() const { return m_configuration.nearTimeUntilTx; }

	void setFarTimeUntilTx(int32_t farTimeUntilTx) { m_configuration.farTimeUntilTx = farTimeUntilTx; }

	int32_t getFarTimeUntilTx() const { return m_configuration.farTimeUntilTx; }

	void setTimeUntilIAm(int32_t timeUntilIAm) { m_configuration.timeUntilIAm = timeUntilIAm; }

	int32_t getTimeUntilIAm() const { return m_configuration.timeUntilIAm; }

	void setTimeUntilPing(int32_t timeUntilPing) { m_configuration.timeUntilPing = timeUntilPing; }

	int32_t getTimeUntilPing() const { return m_configuration.timeUntilPing; }

	void setMaxErrorCount(uint32_t maxErrorCount) { m_configuration.maxErrorCount = maxErrorCount; }

	uint32_t getMaxErrorCount() const { return m_configuration.maxErrorCount; }

	void setMaxDeltaStates(uint32_t maxDeltaStates) { m_configuration.maxDeltaStates = maxDeltaStates; }

	uint32_t getMaxDeltaStates() const { return m_configuration.maxDeltaStates; }

	void setMaxExtrapolationDelta(float maxExtrapolationDelta) { m_configuration.maxExtrapolationDelta = maxExtrapolationDelta; }

	float getMaxExtrapolationDelta() const { return m_configuration.maxExtrapolationDelta; }

	void setDeltaCompression(bool deltaCompression) { m_configuration.deltaCompression = deltaCompression; }

	bool getDeltaCompression() const { return m_configuration.deltaCompression; }

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

	virtual void notify(net::Replicator* replicator, float eventTime, uint32_t eventId, net::handle_t peerHandle, const Object* eventObject)
	{
		script::Any argv[] =
		{
			script::CastAny< Object* >::set(replicator),
			script::CastAny< float >::set(eventTime),
			script::CastAny< int32_t >::set(int32_t(eventId)),
			script::CastAny< uint8_t >::set(peerHandle),
			script::CastAny< Object* >::set((Object*)eventObject)
		};
		if (m_delegate)
			m_delegate->call(sizeof_array(argv), argv);
	}

private:
	Ref< script::IScriptDelegate > m_delegate;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.ReplicatorListener", ReplicatorListener, net::Replicator::IListener)

bool net_Replicator_create(net::Replicator* self, net::IReplicatorPeers* replicatorPeers, const ReplicatorConfiguration* configuration)
{
	return self->create(replicatorPeers, configuration ? configuration->getConfiguration() : net::Replicator::Configuration());
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

	Ref< script::AutoScriptClass< net::IReplicatorPeers > > classIReplicatorPeers = new script::AutoScriptClass< net::IReplicatorPeers >();
	classIReplicatorPeers->addMethod("destroy", &net::IReplicatorPeers::destroy);
	scriptManager->registerClass(classIReplicatorPeers);

	Ref< script::AutoScriptClass< net::BandwidthLimitPeers > > classBandwidthLimitPeers = new script::AutoScriptClass< net::BandwidthLimitPeers >();
	classBandwidthLimitPeers->addConstructor< net::IReplicatorPeers*, int32_t, bool, bool >();
	scriptManager->registerClass(classBandwidthLimitPeers);

	Ref< script::AutoScriptClass< net::DiagnosePeers > > classDiagnosePeers = new script::AutoScriptClass< net::DiagnosePeers >();
	classDiagnosePeers->addConstructor< net::IReplicatorPeers* >();
	scriptManager->registerClass(classDiagnosePeers);

	Ref< script::AutoScriptClass< net::InetSimPeers > > classInetSimPeers = new script::AutoScriptClass< net::InetSimPeers >();
	classInetSimPeers->addConstructor< net::IReplicatorPeers* >();
	classInetSimPeers->addMethod("setPeerConnectionState", &net::InetSimPeers::setPeerConnectionState);
	scriptManager->registerClass(classInetSimPeers);

	Ref< script::AutoScriptClass< net::RecordPeers > > classRecordPeers = new script::AutoScriptClass< net::RecordPeers >();
	classRecordPeers->addConstructor< net::IReplicatorPeers*, IStream* >();
	scriptManager->registerClass(classRecordPeers);

	Ref< script::AutoScriptClass< net::RelayPeers > > classRelayPeers = new script::AutoScriptClass< net::RelayPeers >();
	classRelayPeers->addConstructor< net::IReplicatorPeers* >();
	scriptManager->registerClass(classRelayPeers);

	Ref< script::AutoScriptClass< net::ReliableTransportPeers > > classReliableTransportPeers = new script::AutoScriptClass< net::ReliableTransportPeers >();
	classReliableTransportPeers->addConstructor< net::IReplicatorPeers*, bool >();
	scriptManager->registerClass(classReliableTransportPeers);

	Ref< script::AutoScriptClass< net::Replicator::IListener > > classReplicatorIListener = new script::AutoScriptClass< net::Replicator::IListener >();
	scriptManager->registerClass(classReplicatorIListener);

	Ref< script::AutoScriptClass< ReplicatorConfiguration > > classReplicatorConfiguration = new script::AutoScriptClass< ReplicatorConfiguration >();
	classReplicatorConfiguration->addConstructor();
	classReplicatorConfiguration->addMethod("setNearDistance", &ReplicatorConfiguration::setNearDistance);
	classReplicatorConfiguration->addMethod("getNearDistance", &ReplicatorConfiguration::getNearDistance);
	classReplicatorConfiguration->addMethod("setFarDistance", &ReplicatorConfiguration::setFarDistance);
	classReplicatorConfiguration->addMethod("getFarDistance", &ReplicatorConfiguration::getFarDistance);
	classReplicatorConfiguration->addMethod("setNearTimeUntilTx", &ReplicatorConfiguration::setNearTimeUntilTx);
	classReplicatorConfiguration->addMethod("getNearTimeUntilTx", &ReplicatorConfiguration::getNearTimeUntilTx);
	classReplicatorConfiguration->addMethod("setFarTimeUntilTx", &ReplicatorConfiguration::setFarTimeUntilTx);
	classReplicatorConfiguration->addMethod("getFarTimeUntilTx", &ReplicatorConfiguration::getFarTimeUntilTx);
	classReplicatorConfiguration->addMethod("setTimeUntilIAm", &ReplicatorConfiguration::setTimeUntilIAm);
	classReplicatorConfiguration->addMethod("getTimeUntilIAm", &ReplicatorConfiguration::getTimeUntilIAm);
	classReplicatorConfiguration->addMethod("setTimeUntilPing", &ReplicatorConfiguration::setTimeUntilPing);
	classReplicatorConfiguration->addMethod("getTimeUntilPing", &ReplicatorConfiguration::getTimeUntilPing);
	classReplicatorConfiguration->addMethod("setMaxErrorCount", &ReplicatorConfiguration::setMaxErrorCount);
	classReplicatorConfiguration->addMethod("getMaxErrorCount", &ReplicatorConfiguration::getMaxErrorCount);
	classReplicatorConfiguration->addMethod("setMaxDeltaStates", &ReplicatorConfiguration::setMaxDeltaStates);
	classReplicatorConfiguration->addMethod("getMaxDeltaStates", &ReplicatorConfiguration::getMaxDeltaStates);
	classReplicatorConfiguration->addMethod("setMaxExtrapolationDelta", &ReplicatorConfiguration::setMaxExtrapolationDelta);
	classReplicatorConfiguration->addMethod("getMaxExtrapolationDelta", &ReplicatorConfiguration::getMaxExtrapolationDelta);
	classReplicatorConfiguration->addMethod("setDeltaCompression", &ReplicatorConfiguration::setDeltaCompression);
	classReplicatorConfiguration->addMethod("getDeltaCompression", &ReplicatorConfiguration::getDeltaCompression);
	scriptManager->registerClass(classReplicatorConfiguration);

	Ref< script::AutoScriptClass< ReplicatorListener > > classReplicatorListener = new script::AutoScriptClass< ReplicatorListener >();
	classReplicatorListener->addConstructor< script::IScriptDelegate* >();
	scriptManager->registerClass(classReplicatorListener);

	Ref< script::AutoScriptClass< net::Replicator > > classReplicator = new script::AutoScriptClass< net::Replicator >();
	classReplicator->addConstructor();
	classReplicator->addMethod("create", &net_Replicator_create);
	classReplicator->addMethod("destroy", &net::Replicator::destroy);
	classReplicator->addMethod("setConfiguration", &net_Replicator_setConfiguration);
	classReplicator->addMethod("getConfiguration", &net_Replicator_getConfiguration);
	classReplicator->addMethod("addEventType", &net::Replicator::addEventType);
	classReplicator->addMethod("addListener", &net::Replicator::addListener);
	classReplicator->addMethod("update", &net::Replicator::update);
	classReplicator->addMethod("getHandle", &net::Replicator::getHandle);
	classReplicator->addMethod("getName", &net::Replicator::getName);
	classReplicator->addMethod("setStatus", &net::Replicator::setStatus);
	classReplicator->addMethod("setOrigin", &net::Replicator::setOrigin);
	classReplicator->addMethod("setStateTemplate", &net::Replicator::setStateTemplate);
	classReplicator->addMethod("setState", &net::Replicator::setState);
	classReplicator->addMethod("sendEvent", &net::Replicator::sendEvent);
	classReplicator->addMethod("broadcastEvent", &net::Replicator::broadcastEvent);
	classReplicator->addMethod("isPrimary", &net::Replicator::isPrimary);
	classReplicator->addMethod("getPeerCount", &net::Replicator::getPeerCount);
	classReplicator->addMethod("getPeerHandle", &net::Replicator::getPeerHandle);
	classReplicator->addMethod("getPeerName", &net::Replicator::getPeerName);
	classReplicator->addMethod("getPeerEndSite", &net::Replicator::getPeerEndSite);
	classReplicator->addMethod("getPeerStatus", &net::Replicator::getPeerStatus);
	classReplicator->addMethod("getPeerLatency", &net::Replicator::getPeerLatency);
	classReplicator->addMethod("getPeerReversedLatency", &net::Replicator::getPeerReversedLatency);
	classReplicator->addMethod("getBestReversedLatency", &net::Replicator::getBestReversedLatency);
	classReplicator->addMethod("getWorstReversedLatency", &net::Replicator::getWorstReversedLatency);
	classReplicator->addMethod("isPeerConnected", &net::Replicator::isPeerConnected);
	classReplicator->addMethod("isPeerRelayed", &net::Replicator::isPeerRelayed);
	classReplicator->addMethod("setPeerPrimary", &net::Replicator::setPeerPrimary);
	classReplicator->addMethod("getPrimaryPeerHandle", &net::Replicator::getPrimaryPeerHandle);
	classReplicator->addMethod("isPeerPrimary", &net::Replicator::isPeerPrimary);
	classReplicator->addMethod("areAllPeersConnected", &net::Replicator::areAllPeersConnected);
	classReplicator->addMethod("setGhostObject", &net::Replicator::setGhostObject);
	classReplicator->addMethod("getGhostObject", &net::Replicator::getGhostObject);
	classReplicator->addMethod("setGhostOrigin", &net::Replicator::setGhostOrigin);
	classReplicator->addMethod("setGhostStateTemplate", &net::Replicator::setGhostStateTemplate);
	classReplicator->addMethod("getGhostStateTemplate", &net::Replicator::getGhostStateTemplate);
	classReplicator->addMethod("getGhostStateTime", &net::Replicator::getGhostStateTime);
	classReplicator->addMethod("getGhostState", &net::Replicator::getGhostState);
	classReplicator->addMethod("getLoopBackState", &net::Replicator::getLoopBackState);
	classReplicator->addMethod("getState", &net::Replicator::getState);
	classReplicator->addMethod("getTime", &net::Replicator::getTime);
	scriptManager->registerClass(classReplicator);
}

	}
}