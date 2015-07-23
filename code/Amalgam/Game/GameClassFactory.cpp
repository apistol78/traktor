#include "Amalgam/Game/GameClassFactory.h"
#include "Amalgam/Game/IEnvironment.h"
#include "Amalgam/Game/IInputServer.h"
#include "Amalgam/Game/IRenderServer.h"
#include "Amalgam/Game/IUpdateControl.h"
#include "Amalgam/Game/IUpdateInfo.h"
#include "Amalgam/Game/Engine/AudioLayer.h"
#include "Amalgam/Game/Engine/StageData.h"
#include "Amalgam/Game/Engine/FlashLayer.h"
#include "Amalgam/Game/Engine/GameEntity.h"
#include "Amalgam/Game/Engine/GameEntityData.h"
#include "Amalgam/Game/Engine/Stage.h"
#include "Amalgam/Game/Engine/StageData.h"
#include "Amalgam/Game/Engine/StageLoader.h"
#include "Amalgam/Game/Engine/VideoLayer.h"
#include "Amalgam/Game/Engine/WorldLayer.h"
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Drawing/Image.h"
#include "World/Entity.h"
#include "World/EntityData.h"
#include "World/IEntityBuilder.h"
#include "World/IEntityFactory.h"
#include "World/IEntityRenderer.h"
#include "World/IEntitySchema.h"
#include "World/IWorldRenderer.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldRenderSettings.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

class BoxedTransition : public Object
{
	T_RTTI_CLASS;

public:
	BoxedTransition(const std::wstring& id, const Guid& reference)
	:	m_id(id)
	,	m_reference(reference)
	{
	}

	const std::wstring& getId() const { return m_id; }

	const Guid& getReference() const { return m_reference; }

private:
	std::wstring m_id;
	Guid m_reference;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.Transition", BoxedTransition, Object)

RefArray< BoxedTransition > StageData_getTransitions(StageData* self)
{
	RefArray< BoxedTransition > out;

	const std::map< std::wstring, Guid >& transitions = self->getTransitions();
	for (std::map< std::wstring, Guid >::const_iterator i = transitions.begin(); i != transitions.end(); ++i)
		out.push_back(new BoxedTransition(i->first, i->second));

	return out;
}

Ref< flash::ActionObject > FlashLayer_createObject_0(FlashLayer* self)
{
	return self->createObject();
}

Ref< flash::ActionObject > FlashLayer_createObject_1(FlashLayer* self, uint32_t argc, const Any* argv)
{
	return self->createObject(argc, argv);
}

Ref< world::Entity > WorldLayer_createEntity1(WorldLayer* self, const std::wstring& name, world::IEntitySchema* entitySchema)
{
	return self->createEntity(name, entitySchema);
}

Ref< world::Entity > WorldLayer_createEntity2(WorldLayer* self, const std::wstring& name)
{
	return self->createEntity(name, 0);
}

Ref< BoxedVector4 > WorldLayer_worldToView(WorldLayer* self, const Vector4& worldPosition)
{
	Vector4 viewPosition;
	if (self->worldToView(worldPosition, viewPosition))
		return new BoxedVector4(viewPosition);
	else
		return 0;
}

Ref< BoxedVector4 > WorldLayer_viewToWorld(WorldLayer* self, const Vector4& viewPosition)
{
	Vector4 worldPosition;
	if (self->viewToWorld(viewPosition, worldPosition))
		return new BoxedVector4(worldPosition);
	else
		return 0;
}

Ref< BoxedVector2 > WorldLayer_worldToScreen(WorldLayer* self, const Vector4& worldPosition)
{
	Vector2 screenPosition;
	if (self->worldToScreen(worldPosition, screenPosition))
		return new BoxedVector2(screenPosition);
	else
		return 0;
}

Ref< BoxedVector2 > WorldLayer_viewToScreen(WorldLayer* self, const Vector4& viewPosition)
{
	Vector2 screenPosition;
	if (self->viewToScreen(viewPosition, screenPosition))
		return new BoxedVector2(screenPosition);
	else
		return 0;
}

world::IEntityEventInstance* GameEntity_raiseEvent_1(GameEntity* self, const std::wstring& eventId)
{
	return self->raiseEvent(eventId);
}

world::IEntityEventInstance* GameEntity_raiseEvent_2(GameEntity* self, const std::wstring& eventId, const Transform& Toffset)
{
	return self->raiseEvent(eventId, Toffset);
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.GameClassFactory", 0, GameClassFactory, IRuntimeClassFactory)

void GameClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< amalgam::IEnvironment > > classEnvironment = new AutoRuntimeClass< amalgam::IEnvironment >();
	classEnvironment->addMethod("getDatabase", &amalgam::IEnvironment::getDatabase);
	classEnvironment->addMethod("getControl", &amalgam::IEnvironment::getControl);
	classEnvironment->addMethod("getAudio", &amalgam::IEnvironment::getAudio);
	classEnvironment->addMethod("getInput", &amalgam::IEnvironment::getInput);
	classEnvironment->addMethod("getOnline", &amalgam::IEnvironment::getOnline);
	classEnvironment->addMethod("getPhysics", &amalgam::IEnvironment::getPhysics);
	classEnvironment->addMethod("getRender", &amalgam::IEnvironment::getRender);
	classEnvironment->addMethod("getResource", &amalgam::IEnvironment::getResource);
	classEnvironment->addMethod("getScript", &amalgam::IEnvironment::getScript);
	classEnvironment->addMethod("getWorld", &amalgam::IEnvironment::getWorld);
	classEnvironment->addMethod("getSettings", &amalgam::IEnvironment::getSettings);
	classEnvironment->addMethod("reconfigure", &amalgam::IEnvironment::reconfigure);
	registrar->registerClass(classEnvironment);

	Ref< AutoRuntimeClass< amalgam::IAudioServer > > classAudioServer = new AutoRuntimeClass< amalgam::IAudioServer >();
	classAudioServer->addMethod("getSoundSystem", &amalgam::IAudioServer::getSoundSystem);
	classAudioServer->addMethod("getSoundPlayer", &amalgam::IAudioServer::getSoundPlayer);
	classAudioServer->addMethod("getSurroundEnvironment", &amalgam::IAudioServer::getSurroundEnvironment);
	registrar->registerClass(classAudioServer);

	Ref< AutoRuntimeClass< amalgam::IInputServer > > classInputServer = new AutoRuntimeClass< amalgam::IInputServer >();
	classInputServer->addMethod("isFabricating", &amalgam::IInputServer::isFabricating);
	classInputServer->addMethod("abortedFabricating", &amalgam::IInputServer::abortedFabricating);
	classInputServer->addMethod("resetInputSource", &amalgam::IInputServer::resetInputSource);
	classInputServer->addMethod("isIdle", &amalgam::IInputServer::isIdle);
	classInputServer->addMethod("getInputSystem", &amalgam::IInputServer::getInputSystem);
	classInputServer->addMethod("getInputMapping", &amalgam::IInputServer::getInputMapping);
	classInputServer->addMethod("getRumbleEffectPlayer", &amalgam::IInputServer::getRumbleEffectPlayer);
	registrar->registerClass(classInputServer);

	Ref< AutoRuntimeClass< amalgam::IOnlineServer > > classOnlineServer = new AutoRuntimeClass< amalgam::IOnlineServer >();
	classOnlineServer->addMethod("getSessionManager", &amalgam::IOnlineServer::getSessionManager);
	registrar->registerClass(classOnlineServer);

	Ref< AutoRuntimeClass< amalgam::IPhysicsServer > > classPhysicsServer = new AutoRuntimeClass< amalgam::IPhysicsServer >();
	classPhysicsServer->addMethod("getPhysicsManager", &amalgam::IPhysicsServer::getPhysicsManager);
	registrar->registerClass(classPhysicsServer);

	Ref< AutoRuntimeClass< amalgam::IRenderServer > > classRenderServer = new AutoRuntimeClass< amalgam::IRenderServer >();
	classRenderServer->addMethod("getRenderSystem", &amalgam::IRenderServer::getRenderSystem);
	classRenderServer->addMethod("getRenderView", &amalgam::IRenderServer::getRenderView);
	classRenderServer->addMethod("getScreenAspectRatio", &amalgam::IRenderServer::getScreenAspectRatio);
	classRenderServer->addMethod("getViewAspectRatio", &amalgam::IRenderServer::getViewAspectRatio);
	classRenderServer->addMethod("getAspectRatio", &amalgam::IRenderServer::getAspectRatio);
	classRenderServer->addMethod("getStereoscopic", &amalgam::IRenderServer::getStereoscopic);
	classRenderServer->addMethod("getMultiSample", &amalgam::IRenderServer::getMultiSample);
	classRenderServer->addMethod("getFrameRate", &amalgam::IRenderServer::getFrameRate);
	registrar->registerClass(classRenderServer);

	Ref< AutoRuntimeClass< amalgam::IResourceServer > > classResourceServer = new AutoRuntimeClass< amalgam::IResourceServer >();
	classResourceServer->addMethod("getResourceManager", &amalgam::IResourceServer::getResourceManager);
	registrar->registerClass(classResourceServer);

	Ref< AutoRuntimeClass< amalgam::IWorldServer > > classWorldServer = new AutoRuntimeClass< amalgam::IWorldServer >();
	classWorldServer->addMethod("addEntityFactory", &amalgam::IWorldServer::addEntityFactory);
	classWorldServer->addMethod("removeEntityFactory", &amalgam::IWorldServer::removeEntityFactory);
	classWorldServer->addMethod("addEntityRenderer", &amalgam::IWorldServer::addEntityRenderer);
	classWorldServer->addMethod("removeEntityRenderer", &amalgam::IWorldServer::removeEntityRenderer);
	classWorldServer->addMethod("getEntityBuilder", &amalgam::IWorldServer::getEntityBuilder);
	classWorldServer->addMethod("getEntityRenderers", &amalgam::IWorldServer::getEntityRenderers);
	classWorldServer->addMethod("getEntityEventManager", &amalgam::IWorldServer::getEntityEventManager);
	classWorldServer->addMethod("getFrameCount", &amalgam::IWorldServer::getFrameCount);
	registrar->registerClass(classWorldServer);

	Ref< AutoRuntimeClass< amalgam::IUpdateControl > > classIUpdateControl = new AutoRuntimeClass< amalgam::IUpdateControl >();
	classIUpdateControl->addMethod("setPause", &amalgam::IUpdateControl::setPause);
	classIUpdateControl->addMethod("getPause", &amalgam::IUpdateControl::getPause);
	classIUpdateControl->addMethod("setTimeScale", &amalgam::IUpdateControl::setTimeScale);
	classIUpdateControl->addMethod("getTimeScale", &amalgam::IUpdateControl::getTimeScale);
	classIUpdateControl->addMethod("setSimulationFrequency", &amalgam::IUpdateControl::setSimulationFrequency);
	classIUpdateControl->addMethod("getSimulationFrequency", &amalgam::IUpdateControl::getSimulationFrequency);
	registrar->registerClass(classIUpdateControl);

	Ref< AutoRuntimeClass< amalgam::IUpdateInfo > > classIUpdateInfo = new AutoRuntimeClass< amalgam::IUpdateInfo >();
	classIUpdateInfo->addMethod("getTotalTime", &amalgam::IUpdateInfo::getTotalTime);
	classIUpdateInfo->addMethod("getStateTime", &amalgam::IUpdateInfo::getStateTime);
	classIUpdateInfo->addMethod("getSimulationTime", &amalgam::IUpdateInfo::getSimulationTime);
	classIUpdateInfo->addMethod("getSimulationDeltaTime", &amalgam::IUpdateInfo::getSimulationDeltaTime);
	classIUpdateInfo->addMethod("getSimulationFrequency", &amalgam::IUpdateInfo::getSimulationFrequency);
	classIUpdateInfo->addMethod("getFrameDeltaTime", &amalgam::IUpdateInfo::getFrameDeltaTime);
	classIUpdateInfo->addMethod("getInterval", &amalgam::IUpdateInfo::getInterval);
	classIUpdateInfo->addMethod("isRunningSlow", &amalgam::IUpdateInfo::isRunningSlow);
	registrar->registerClass(classIUpdateInfo);

	Ref< AutoRuntimeClass< BoxedTransition > > classBoxedTransition = new AutoRuntimeClass< BoxedTransition >();
	classBoxedTransition->addMethod("getId", &BoxedTransition::getId);
	classBoxedTransition->addMethod("getReference", &BoxedTransition::getReference);
	registrar->registerClass(classBoxedTransition);

	Ref< AutoRuntimeClass< StageData > > classStageData = new AutoRuntimeClass< StageData >();
	classStageData->addMethod("getTransitions", &StageData_getTransitions);
	registrar->registerClass(classStageData);

	Ref< AutoRuntimeClass< Stage > > classStage = new AutoRuntimeClass< Stage >();
	classStage->addMethod("addLayer", &Stage::addLayer);
	classStage->addMethod("removeLayer", &Stage::removeLayer);
	classStage->addMethod("removeAllLayers", &Stage::removeAllLayers);
	classStage->addMethod("getLayer", &Stage::getLayer);
	classStage->addMethod("terminate", &Stage::terminate);
	classStage->addMethod("loadStage", &Stage::loadStage);
	classStage->addMethod("loadStageAsync", &Stage::loadStageAsync);
	classStage->addMethod("gotoStage", &Stage::gotoStage);
	classStage->addMethod("getEnvironment", &Stage::getEnvironment);
	classStage->addMethod("getLayers", &Stage::getLayers);
	registrar->registerClass(classStage);

	Ref< AutoRuntimeClass< Layer > > classLayer = new AutoRuntimeClass< Layer >();
	classLayer->addMethod("getName", &Layer::getName);
	registrar->registerClass(classLayer);

	Ref< AutoRuntimeClass< AudioLayer > > classAudioLayer = new AutoRuntimeClass< AudioLayer >();
	classAudioLayer->addMethod("play", &AudioLayer::play);
	classAudioLayer->addMethod("stop", &AudioLayer::stop);
	classAudioLayer->addMethod("fadeOff", &AudioLayer::fadeOff);
	classAudioLayer->addMethod("setParameter", &AudioLayer::setParameter);
	classAudioLayer->addMethod("tweenParameter", &AudioLayer::tweenParameter);
	registrar->registerClass(classAudioLayer);

	Ref< AutoRuntimeClass< FlashLayer > > classFlashLayer = new AutoRuntimeClass< FlashLayer >();
	classFlashLayer->addMethod("getMoviePlayer", &FlashLayer::getMoviePlayer);
	classFlashLayer->addMethod("getGlobal", &FlashLayer::getGlobal);
	classFlashLayer->addMethod("getRoot", &FlashLayer::getRoot);
	classFlashLayer->addMethod("createObject", &FlashLayer_createObject_0);
	classFlashLayer->addVariadicMethod("createObject", &FlashLayer_createObject_1);
	classFlashLayer->addMethod("createBitmap", &FlashLayer::createBitmap);
	classFlashLayer->addMethod("isVisible", &FlashLayer::isVisible);
	classFlashLayer->addMethod("setVisible", &FlashLayer::setVisible);
	classFlashLayer->addMethod("getPrintableString", &FlashLayer::getPrintableString);
	classFlashLayer->setUnknownMethod(&FlashLayer::externalCall);
	registrar->registerClass(classFlashLayer);

	Ref< AutoRuntimeClass< VideoLayer > > classVideoLayer = new AutoRuntimeClass< VideoLayer >();
	classVideoLayer->addMethod("play", &VideoLayer::play);
	classVideoLayer->addMethod("stop", &VideoLayer::stop);
	classVideoLayer->addMethod("rewind", &VideoLayer::rewind);
	classVideoLayer->addMethod("show", &VideoLayer::show);
	classVideoLayer->addMethod("hide", &VideoLayer::hide);
	classVideoLayer->addMethod("isPlaying", &VideoLayer::isPlaying);
	classVideoLayer->addMethod("isVisible", &VideoLayer::isVisible);
	classVideoLayer->addMethod("setScreenBounds", &VideoLayer::setScreenBounds);
	classVideoLayer->addMethod("getScreenBounds", &VideoLayer::getScreenBounds);
	classVideoLayer->addMethod("setRepeat", &VideoLayer::setRepeat);
	classVideoLayer->addMethod("getRepeat", &VideoLayer::getRepeat);
	registrar->registerClass(classVideoLayer);

	Ref< AutoRuntimeClass< WorldLayer > > classWorldLayer = new AutoRuntimeClass< WorldLayer >();
	classWorldLayer->addMethod("getEntityData", &WorldLayer::getEntityData);
	classWorldLayer->addMethod("getEntity", &WorldLayer::getEntity);
	classWorldLayer->addMethod("getEntities", &WorldLayer::getEntities);
	classWorldLayer->addMethod("getEntitiesOf", &WorldLayer::getEntitiesOf);
	classWorldLayer->addMethod("createEntity", &WorldLayer_createEntity1);
	classWorldLayer->addMethod("createEntity", &WorldLayer_createEntity2);
	classWorldLayer->addMethod("getEntityIndex", &WorldLayer::getEntityIndex);
	classWorldLayer->addMethod("getEntityIndexOf", &WorldLayer::getEntityIndexOf);
	classWorldLayer->addMethod("getEntityByIndex", &WorldLayer::getEntityByIndex);
	classWorldLayer->addMethod("getEntityOf", &WorldLayer::getEntityOf);
	classWorldLayer->addMethod("addEntity", &WorldLayer::addEntity);
	classWorldLayer->addMethod("addTransientEntity", &WorldLayer::addTransientEntity);
	classWorldLayer->addMethod("removeEntity", &WorldLayer::removeEntity);
	classWorldLayer->addMethod("isEntityAdded", &WorldLayer::isEntityAdded);
	classWorldLayer->addMethod("getEntitySchema", &WorldLayer::getEntitySchema);
	classWorldLayer->addMethod("setControllerEnable", &WorldLayer::setControllerEnable);
	classWorldLayer->addMethod("resetController", &WorldLayer::resetController);
	classWorldLayer->addMethod("getPostProcess", &WorldLayer::getPostProcess);
	classWorldLayer->addMethod("getViewFrustum", &WorldLayer::getViewFrustum);
	classWorldLayer->addMethod("worldToView", &WorldLayer_worldToView);
	classWorldLayer->addMethod("viewToWorld", &WorldLayer_viewToWorld);
	classWorldLayer->addMethod("worldToScreen", &WorldLayer_worldToScreen);
	classWorldLayer->addMethod("viewToScreen", &WorldLayer_viewToScreen);
	classWorldLayer->addMethod("setFieldOfView", &WorldLayer::setFieldOfView);
	classWorldLayer->addMethod("getFieldOfView", &WorldLayer::getFieldOfView);
	classWorldLayer->addMethod("setAlternateTime", &WorldLayer::setAlternateTime);
	classWorldLayer->addMethod("getAlternateTime", &WorldLayer::getAlternateTime);
	classWorldLayer->addMethod("setFeedbackScale", &WorldLayer::setFeedbackScale);
	classWorldLayer->addMethod("getFeedbackScale", &WorldLayer::getFeedbackScale);
	classWorldLayer->addMethod("setCamera", &WorldLayer::setCamera);
	classWorldLayer->addMethod("getCamera", &WorldLayer::getCamera);
	classWorldLayer->addMethod("setListener", &WorldLayer::setListener);
	classWorldLayer->addMethod("getListener", &WorldLayer::getListener);
	registrar->registerClass(classWorldLayer);

	Ref< AutoRuntimeClass< StageLoader > > classStageLoader = new AutoRuntimeClass< StageLoader >();
	classStageLoader->addMethod("wait", &StageLoader::wait);
	classStageLoader->addMethod("ready", &StageLoader::ready);
	classStageLoader->addMethod("succeeded", &StageLoader::succeeded);
	classStageLoader->addMethod("failed", &StageLoader::failed);
	classStageLoader->addMethod("get", &StageLoader::get);
	registrar->registerClass(classStageLoader);

	Ref< AutoRuntimeClass< GameEntityData > > classGameEntityData = new AutoRuntimeClass< GameEntityData >();
	registrar->registerClass(classGameEntityData);

	Ref< AutoRuntimeClass< GameEntity > > classGameEntity = new AutoRuntimeClass< GameEntity >();
	classGameEntity->addMethod("raiseEvent", &GameEntity_raiseEvent_1);
	classGameEntity->addMethod("raiseEvent", &GameEntity_raiseEvent_2);
	classGameEntity->addMethod("setTag", &GameEntity::setTag);
	classGameEntity->addMethod("getTag", &GameEntity::getTag);
	classGameEntity->addMethod("setObject", &GameEntity::setObject);
	classGameEntity->addMethod("getObject", &GameEntity::getObject);
	classGameEntity->addMethod("setEntity", &GameEntity::setEntity);
	classGameEntity->addMethod("getEntity", &GameEntity::getEntity);
	classGameEntity->addMethod("setVisible", &GameEntity::setVisible);
	classGameEntity->addMethod("isVisible", &GameEntity::isVisible);
	registrar->registerClass(classGameEntity);
}

	}
}
