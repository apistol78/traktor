#include "Online/ILobby.h"
#include "Online/ISessionManager.h"
#include "Parade/AudioLayer.h"
#include "Parade/Stage.h"
#include "Parade/VideoLayer.h"
#include "Parade/WorldLayer.h"
#include "Parade/Classes/ParadeClasses.h"
#include "Parade/Network/InetSimPeers.h"
#include "Parade/Network/IReplicatableState.h"
#include "Parade/Network/LanReplicatorPeers.h"
#include "Parade/Network/OnlineReplicatorPeers.h"
#include "Parade/Network/PropertyState.h"
#include "Parade/Network/Replicator.h"
#include "Script/AutoScriptClass.h"
#include "Script/Boxes.h"
#include "Script/Delegate.h"
#include "Script/IScriptManager.h"
#include "World/Entity/IEntityFactory.h"
#include "World/Entity/IEntityRenderer.h"
#include "World/Entity/IEntitySchema.h"
#include "World/Entity/Entity.h"

namespace traktor
{
	namespace parade
	{
		namespace
		{

class ReplicatorListener : public Replicator::IListener
{
	T_RTTI_CLASS;

public:
	ReplicatorListener(script::Delegate* delegate)
	:	m_delegate(delegate)
	{
	}

	virtual void notify(Replicator* replicator, float eventTime, uint32_t eventId, uint32_t peerId, const Object* eventObject)
	{
		script::Any argv[] =
		{
			script::Any(replicator),
			script::Any(eventTime),
			script::Any(int32_t(eventId)),
			script::Any(int32_t(peerId)),
			script::Any((Object*)eventObject)
		};
		if (m_delegate)
			m_delegate->invoke(sizeof_array(argv), argv);
	}

private:
	Ref< script::Delegate > m_delegate;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.ReplicatorListener", ReplicatorListener, Replicator::IListener)

Ref< world::Entity > WorldLayer_createEntity1(WorldLayer* self, const std::wstring& name, world::IEntitySchema* entitySchema)
{
	return self->createEntity(name, entitySchema);
}

Ref< world::Entity > WorldLayer_createEntity2(WorldLayer* self, const std::wstring& name)
{
	return self->createEntity(name, 0);
}

Ref< script::BoxedVector4 > WorldLayer_getViewPosition(WorldLayer* self, const Vector4& worldPosition)
{
	Vector4 viewPosition;
	if (self->getViewPosition(worldPosition, viewPosition))
		return new script::BoxedVector4(viewPosition);
	else
		return 0;
}

Ref< script::BoxedVector2 > WorldLayer_getScreenPosition(WorldLayer* self, const Vector4& viewPosition)
{
	Vector2 screenPosition;
	if (self->getScreenPosition(viewPosition, screenPosition))
		return new script::BoxedVector2(screenPosition);
	else
		return 0;
}

	}

void registerParadeClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< Stage > > classStage = new script::AutoScriptClass< Stage >();
	classStage->addMethod(L"addLayer", &Stage::addLayer);
	classStage->addMethod(L"removeLayer", &Stage::removeLayer);
	classStage->addMethod(L"removeAllLayers", &Stage::removeAllLayers);
	classStage->addMethod(L"findLayer", &Stage::findLayer);
	classStage->addMethod(L"terminate", &Stage::terminate);
	classStage->addMethod(L"gotoStage", &Stage::gotoStage);
	classStage->addMethod(L"getEnvironment", &Stage::getEnvironment);
	classStage->addMethod(L"getLayers", &Stage::getLayers);
	scriptManager->registerClass(classStage);

	Ref< script::AutoScriptClass< Layer > > classLayer = new script::AutoScriptClass< Layer >();
	classLayer->addMethod(L"getName", &Layer::getName);
	scriptManager->registerClass(classLayer);

	Ref< script::AutoScriptClass< AudioLayer > > classAudioLayer = new script::AutoScriptClass< AudioLayer >();
	scriptManager->registerClass(classAudioLayer);

	Ref< script::AutoScriptClass< VideoLayer > > classVideoLayer = new script::AutoScriptClass< VideoLayer >();
	scriptManager->registerClass(classVideoLayer);

	Ref< script::AutoScriptClass< WorldLayer > > classWorldLayer = new script::AutoScriptClass< WorldLayer >();
	classWorldLayer->addMethod(L"addEntityFactory", &WorldLayer::addEntityFactory);
	classWorldLayer->addMethod(L"addEntityRenderer", &WorldLayer::addEntityRenderer);
	classWorldLayer->addMethod(L"getEntity", &WorldLayer::getEntity);
	classWorldLayer->addMethod(L"getEntities", &WorldLayer::getEntities);
	classWorldLayer->addMethod(L"getEntitiesOf", &WorldLayer::getEntitiesOf);
	classWorldLayer->addMethod(L"createEntity", &WorldLayer_createEntity1);
	classWorldLayer->addMethod(L"createEntity", &WorldLayer_createEntity2);
	classWorldLayer->addMethod(L"addEntity", &WorldLayer::addEntity);
	classWorldLayer->addMethod(L"addTransientEntity", &WorldLayer::addTransientEntity);
	classWorldLayer->addMethod(L"removeEntity", &WorldLayer::removeEntity);
	classWorldLayer->addMethod(L"getEntitySchema", &WorldLayer::getEntitySchema);
	classWorldLayer->addMethod(L"setControllerEnable", &WorldLayer::setControllerEnable);
	classWorldLayer->addMethod(L"getViewPosition", &WorldLayer_getViewPosition);
	classWorldLayer->addMethod(L"getScreenPosition", &WorldLayer_getScreenPosition);
	scriptManager->registerClass(classWorldLayer);

	Ref< script::AutoScriptClass< IReplicatableState > > classReplicatableState = new script::AutoScriptClass< IReplicatableState >();
	classReplicatableState->addMethod(L"extrapolate", &IReplicatableState::extrapolate);
	scriptManager->registerClass(classReplicatableState);

	Ref< script::AutoScriptClass< PropertyState > > classPropertyState = new script::AutoScriptClass< PropertyState >();
	classPropertyState->addConstructor();
	classPropertyState->addMethod(L"packFloat", &PropertyState::packFloat);
	classPropertyState->addMethod(L"packVector3", &PropertyState::packVector3);
	classPropertyState->addMethod(L"packVector4", &PropertyState::packVector4);
	classPropertyState->addMethod(L"packQuaternion", &PropertyState::packQuaternion);
	classPropertyState->addMethod(L"packTransform", &PropertyState::packTransform);
	classPropertyState->addMethod(L"unpackBegin", &PropertyState::unpackBegin);
	classPropertyState->addMethod(L"unpackFloat", &PropertyState::unpackFloat);
	classPropertyState->addMethod(L"unpackVector3", &PropertyState::unpackVector3);
	classPropertyState->addMethod(L"unpackVector4", &PropertyState::unpackVector4);
	classPropertyState->addMethod(L"unpackQuaternion", &PropertyState::unpackQuaternion);
	classPropertyState->addMethod(L"unpackTransform", &PropertyState::unpackTransform);
	scriptManager->registerClass(classPropertyState);

	Ref< script::AutoScriptClass< IReplicatorPeers > > classIReplicatorPeers = new script::AutoScriptClass< IReplicatorPeers >();
	classIReplicatorPeers->addMethod(L"destroy", &IReplicatorPeers::destroy);
	scriptManager->registerClass(classIReplicatorPeers);

	Ref< script::AutoScriptClass< InetSimPeers > > classInetSimPeers = new script::AutoScriptClass< InetSimPeers >();
	classInetSimPeers->addConstructor< IReplicatorPeers*, float, float, float >();
	scriptManager->registerClass(classInetSimPeers);

	Ref< script::AutoScriptClass< LanReplicatorPeers > > classLanReplicatorPeers = new script::AutoScriptClass< LanReplicatorPeers >();
	classLanReplicatorPeers->addConstructor();
	classLanReplicatorPeers->addMethod(L"create", &LanReplicatorPeers::create);
	scriptManager->registerClass(classLanReplicatorPeers);

	Ref< script::AutoScriptClass< OnlineReplicatorPeers > > classOnlineReplicatorPeers = new script::AutoScriptClass< OnlineReplicatorPeers >();
	classOnlineReplicatorPeers->addConstructor();
	classOnlineReplicatorPeers->addMethod(L"create", &OnlineReplicatorPeers::create);
	scriptManager->registerClass(classOnlineReplicatorPeers);

	Ref< script::AutoScriptClass< Replicator::IListener > > classReplicatorIListener = new script::AutoScriptClass< Replicator::IListener >();
	scriptManager->registerClass(classReplicatorIListener);

	Ref< script::AutoScriptClass< ReplicatorListener > > classReplicatorListener = new script::AutoScriptClass< ReplicatorListener >();
	classReplicatorListener->addConstructor< script::Delegate* >();
	scriptManager->registerClass(classReplicatorListener);

	Ref< script::AutoScriptClass< Replicator > > classReplicator = new script::AutoScriptClass< Replicator >();
	classReplicator->addConstructor();
	classReplicator->addMethod(L"create", &Replicator::create);
	classReplicator->addMethod(L"destroy", &Replicator::destroy);
	classReplicator->addMethod(L"addEventType", &Replicator::addEventType);
	classReplicator->addMethod(L"addListener", &Replicator::addListener);
	classReplicator->addMethod(L"update", &Replicator::update);
	classReplicator->addMethod(L"setOrigin", &Replicator::setOrigin);
	classReplicator->addMethod(L"setState", &Replicator::setState);
	classReplicator->addMethod(L"sendEvent", &Replicator::sendEvent);
	classReplicator->addMethod(L"broadcastEvent", &Replicator::broadcastEvent);
	classReplicator->addMethod(L"getMaxPeerId", &Replicator::getMaxPeerId);
	classReplicator->addMethod(L"setGhostObject", &Replicator::setGhostObject);
	classReplicator->addMethod(L"getGhostObject", &Replicator::getGhostObject);
	classReplicator->addMethod(L"setGhostOrigin", &Replicator::setGhostOrigin);
	classReplicator->addMethod(L"getGhostState", &Replicator::getGhostState);
	classReplicator->addMethod(L"getTime", &Replicator::getTime);
	scriptManager->registerClass(classReplicator);
}

	}
}
