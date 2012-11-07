#include "Online/ILobby.h"
#include "Online/ISessionManager.h"
#include "Net/Replication/DiagnosePeers.h"
#include "Net/Replication/InetSimPeers.h"
#include "Net/Replication/LanReplicatorPeers.h"
#include "Net/Replication/OnlineReplicatorPeers.h"
#include "Net/Replication/ReliableTransportPeers.h"
#include "Net/Replication/Replicator.h"
#include "Net/Replication/State/State.h"
#include "Net/Replication/State/StateTemplate.h"
#include "Script/AutoScriptClass.h"
#include "Script/Boxes.h"
#include "Script/Delegate.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace parade
	{
		namespace
		{

class ReplicatorListener : public net::Replicator::IListener
{
	T_RTTI_CLASS;

public:
	ReplicatorListener(script::Delegate* delegate)
	:	m_delegate(delegate)
	{
	}

	virtual void notify(net::Replicator* replicator, float eventTime, uint32_t eventId, net::handle_t peerHandle, const Object* eventObject)
	{
		script::Any argv[] =
		{
			script::Any(replicator),
			script::Any(eventTime),
			script::Any(int32_t(eventId)),
			script::Any(int32_t(peerHandle)),
			script::Any((Object*)eventObject)
		};
		if (m_delegate)
			m_delegate->invoke(sizeof_array(argv), argv);
	}

private:
	Ref< script::Delegate > m_delegate;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.ReplicatorListener", ReplicatorListener, net::Replicator::IListener)

		}

void registerNetClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< net::State > > classState = new script::AutoScriptClass< net::State >();
	scriptManager->registerClass(classState);

	Ref< script::AutoScriptClass< net::StateTemplate > > classStateTemplate = new script::AutoScriptClass< net::StateTemplate >();
	scriptManager->registerClass(classStateTemplate);

	Ref< script::AutoScriptClass< net::IReplicatorPeers > > classIReplicatorPeers = new script::AutoScriptClass< net::IReplicatorPeers >();
	classIReplicatorPeers->addMethod(L"destroy", &net::IReplicatorPeers::destroy);
	scriptManager->registerClass(classIReplicatorPeers);

	Ref< script::AutoScriptClass< net::DiagnosePeers > > classDiagnosePeers = new script::AutoScriptClass< net::DiagnosePeers >();
	classDiagnosePeers->addConstructor< net::IReplicatorPeers* >();
	scriptManager->registerClass(classDiagnosePeers);

	Ref< script::AutoScriptClass< net::InetSimPeers > > classInetSimPeers = new script::AutoScriptClass< net::InetSimPeers >();
	classInetSimPeers->addConstructor< net::IReplicatorPeers*, float, float, float >();
	scriptManager->registerClass(classInetSimPeers);

	Ref< script::AutoScriptClass< net::LanReplicatorPeers > > classLanReplicatorPeers = new script::AutoScriptClass< net::LanReplicatorPeers >();
	classLanReplicatorPeers->addConstructor();
	classLanReplicatorPeers->addMethod(L"create", &net::LanReplicatorPeers::create);
	scriptManager->registerClass(classLanReplicatorPeers);

	Ref< script::AutoScriptClass< net::OnlineReplicatorPeers > > classOnlineReplicatorPeers = new script::AutoScriptClass< net::OnlineReplicatorPeers >();
	classOnlineReplicatorPeers->addConstructor();
	classOnlineReplicatorPeers->addMethod(L"create", &net::OnlineReplicatorPeers::create);
	scriptManager->registerClass(classOnlineReplicatorPeers);

	Ref< script::AutoScriptClass< net::ReliableTransportPeers > > classReliableTransportPeers = new script::AutoScriptClass< net::ReliableTransportPeers >();
	classReliableTransportPeers->addConstructor< net::IReplicatorPeers* >();
	scriptManager->registerClass(classReliableTransportPeers);

	Ref< script::AutoScriptClass< net::Replicator::IListener > > classReplicatorIListener = new script::AutoScriptClass< net::Replicator::IListener >();
	scriptManager->registerClass(classReplicatorIListener);

	Ref< script::AutoScriptClass< ReplicatorListener > > classReplicatorListener = new script::AutoScriptClass< ReplicatorListener >();
	classReplicatorListener->addConstructor< script::Delegate* >();
	scriptManager->registerClass(classReplicatorListener);

	Ref< script::AutoScriptClass< net::Replicator > > classReplicator = new script::AutoScriptClass< net::Replicator >();
	classReplicator->addConstructor();
	classReplicator->addMethod(L"create", &net::Replicator::create);
	classReplicator->addMethod(L"destroy", &net::Replicator::destroy);
	classReplicator->addMethod(L"addEventType", &net::Replicator::addEventType);
	classReplicator->addMethod(L"addListener", &net::Replicator::addListener);
	classReplicator->addMethod(L"update", &net::Replicator::update);
	classReplicator->addMethod(L"setOrigin", &net::Replicator::setOrigin);
	classReplicator->addMethod(L"setStateTemplate", &net::Replicator::setStateTemplate);
	classReplicator->addMethod(L"setState", &net::Replicator::setState);
	classReplicator->addMethod(L"sendEvent", &net::Replicator::sendEvent);
	classReplicator->addMethod(L"broadcastEvent", &net::Replicator::broadcastEvent);
	classReplicator->addMethod(L"isPrimary", &net::Replicator::isPrimary);
	classReplicator->addMethod(L"getPeerCount", &net::Replicator::getPeerCount);
	classReplicator->addMethod(L"getPeerHandle", &net::Replicator::getPeerHandle);
	classReplicator->addMethod(L"getPeerName", &net::Replicator::getPeerName);
	classReplicator->addMethod(L"getPeerLatency", &net::Replicator::getPeerLatency);
	classReplicator->addMethod(L"getPeerReversedLatency", &net::Replicator::getPeerReversedLatency);
	classReplicator->addMethod(L"getBestReversedLatency", &net::Replicator::getBestReversedLatency);
	classReplicator->addMethod(L"getWorstReversedLatency", &net::Replicator::getWorstReversedLatency);
	classReplicator->addMethod(L"isPeerConnected", &net::Replicator::isPeerConnected);
	classReplicator->addMethod(L"setGhostObject", &net::Replicator::setGhostObject);
	classReplicator->addMethod(L"getGhostObject", &net::Replicator::getGhostObject);
	classReplicator->addMethod(L"setGhostOrigin", &net::Replicator::setGhostOrigin);
	classReplicator->addMethod(L"setGhostStateTemplate", &net::Replicator::setGhostStateTemplate);
	classReplicator->addMethod(L"getGhostState", &net::Replicator::getGhostState);
	classReplicator->addMethod(L"getLoopBackState", &net::Replicator::getLoopBackState);
	classReplicator->addMethod(L"getState", &net::Replicator::getState);
	classReplicator->addMethod(L"getTime", &net::Replicator::getTime);
	scriptManager->registerClass(classReplicator);
}

	}
}