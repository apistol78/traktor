#include "Amalgam/Game/GameClassFactory.h"
#include "Amalgam/Game/IEnvironment.h"
#include "Amalgam/Game/IInputServer.h"
#include "Amalgam/Game/IRenderServer.h"
#include "Amalgam/Game/UpdateControl.h"
#include "Amalgam/Game/UpdateInfo.h"
#include "Amalgam/Game/Engine/AudioLayer.h"
#include "Amalgam/Game/Engine/StageData.h"
#include "Amalgam/Game/Engine/FlashLayer.h"
#include "Amalgam/Game/Engine/SparkLayer.h"
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

world::Entity* WorldLayer_getEntity_1(WorldLayer* self, const std::wstring& name)
{
	return self->getEntity(name);
}

world::Entity* WorldLayer_getEntity_2(WorldLayer* self, const std::wstring& name, int32_t index)
{
	return self->getEntity(name, index);
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

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.GameClassFactory", 0, GameClassFactory, IRuntimeClassFactory)

void GameClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< IEnvironment > > classEnvironment = new AutoRuntimeClass< IEnvironment >();
	classEnvironment->addMethod("getDatabase", &IEnvironment::getDatabase);
	classEnvironment->addMethod("getControl", &IEnvironment::getControl);
	classEnvironment->addMethod("getAudio", &IEnvironment::getAudio);
	classEnvironment->addMethod("getInput", &IEnvironment::getInput);
	classEnvironment->addMethod("getOnline", &IEnvironment::getOnline);
	classEnvironment->addMethod("getPhysics", &IEnvironment::getPhysics);
	classEnvironment->addMethod("getRender", &IEnvironment::getRender);
	classEnvironment->addMethod("getResource", &IEnvironment::getResource);
	classEnvironment->addMethod("getScript", &IEnvironment::getScript);
	classEnvironment->addMethod("getWorld", &IEnvironment::getWorld);
	classEnvironment->addMethod("getSettings", &IEnvironment::getSettings);
	classEnvironment->addMethod("reconfigure", &IEnvironment::reconfigure);
	registrar->registerClass(classEnvironment);

	Ref< AutoRuntimeClass< IAudioServer > > classAudioServer = new AutoRuntimeClass< IAudioServer >();
	classAudioServer->addMethod("getSoundSystem", &IAudioServer::getSoundSystem);
	classAudioServer->addMethod("getSoundPlayer", &IAudioServer::getSoundPlayer);
	classAudioServer->addMethod("getSurroundEnvironment", &IAudioServer::getSurroundEnvironment);
	registrar->registerClass(classAudioServer);

	Ref< AutoRuntimeClass< IInputServer > > classInputServer = new AutoRuntimeClass< IInputServer >();
	classInputServer->addMethod("isFabricating", &IInputServer::isFabricating);
	classInputServer->addMethod("abortedFabricating", &IInputServer::abortedFabricating);
	classInputServer->addMethod("resetInputSource", &IInputServer::resetInputSource);
	classInputServer->addMethod("isIdle", &IInputServer::isIdle);
	classInputServer->addMethod("getInputSystem", &IInputServer::getInputSystem);
	classInputServer->addMethod("getInputMapping", &IInputServer::getInputMapping);
	classInputServer->addMethod("getRumbleEffectPlayer", &IInputServer::getRumbleEffectPlayer);
	registrar->registerClass(classInputServer);

	Ref< AutoRuntimeClass< IOnlineServer > > classOnlineServer = new AutoRuntimeClass< IOnlineServer >();
	classOnlineServer->addMethod("getSessionManager", &IOnlineServer::getSessionManager);
	registrar->registerClass(classOnlineServer);

	Ref< AutoRuntimeClass< IPhysicsServer > > classPhysicsServer = new AutoRuntimeClass< IPhysicsServer >();
	classPhysicsServer->addMethod("getPhysicsManager", &IPhysicsServer::getPhysicsManager);
	registrar->registerClass(classPhysicsServer);

	Ref< AutoRuntimeClass< IRenderServer > > classRenderServer = new AutoRuntimeClass< IRenderServer >();
	classRenderServer->addMethod("getRenderSystem", &IRenderServer::getRenderSystem);
	classRenderServer->addMethod("getRenderView", &IRenderServer::getRenderView);
	classRenderServer->addMethod("getScreenAspectRatio", &IRenderServer::getScreenAspectRatio);
	classRenderServer->addMethod("getViewAspectRatio", &IRenderServer::getViewAspectRatio);
	classRenderServer->addMethod("getAspectRatio", &IRenderServer::getAspectRatio);
	classRenderServer->addMethod("getStereoscopic", &IRenderServer::getStereoscopic);
	classRenderServer->addMethod("getMultiSample", &IRenderServer::getMultiSample);
	classRenderServer->addMethod("getFrameRate", &IRenderServer::getFrameRate);
	registrar->registerClass(classRenderServer);

	Ref< AutoRuntimeClass< IResourceServer > > classResourceServer = new AutoRuntimeClass< IResourceServer >();
	classResourceServer->addMethod("getResourceManager", &IResourceServer::getResourceManager);
	registrar->registerClass(classResourceServer);

	Ref< AutoRuntimeClass< IWorldServer > > classWorldServer = new AutoRuntimeClass< IWorldServer >();
	classWorldServer->addMethod("addEntityFactory", &IWorldServer::addEntityFactory);
	classWorldServer->addMethod("removeEntityFactory", &IWorldServer::removeEntityFactory);
	classWorldServer->addMethod("addEntityRenderer", &IWorldServer::addEntityRenderer);
	classWorldServer->addMethod("removeEntityRenderer", &IWorldServer::removeEntityRenderer);
	classWorldServer->addMethod("getEntityBuilder", &IWorldServer::getEntityBuilder);
	classWorldServer->addMethod("getEntityRenderers", &IWorldServer::getEntityRenderers);
	classWorldServer->addMethod("getEntityEventManager", &IWorldServer::getEntityEventManager);
	registrar->registerClass(classWorldServer);

	Ref< AutoRuntimeClass< UpdateControl > > classUpdateControl = new AutoRuntimeClass< UpdateControl >();
	classUpdateControl->addMethod("setPause", &UpdateControl::setPause);
	classUpdateControl->addMethod("getPause", &UpdateControl::getPause);
	classUpdateControl->addMethod("setTimeScale", &UpdateControl::setTimeScale);
	classUpdateControl->addMethod("getTimeScale", &UpdateControl::getTimeScale);
	classUpdateControl->addMethod("setSimulationFrequency", &UpdateControl::setSimulationFrequency);
	classUpdateControl->addMethod("getSimulationFrequency", &UpdateControl::getSimulationFrequency);
	registrar->registerClass(classUpdateControl);

	Ref< AutoRuntimeClass< UpdateInfo > > classUpdateInfo = new AutoRuntimeClass< UpdateInfo >();
	classUpdateInfo->addMethod("getTotalTime", &UpdateInfo::getTotalTime);
	classUpdateInfo->addMethod("getStateTime", &UpdateInfo::getStateTime);
	classUpdateInfo->addMethod("getSimulationTime", &UpdateInfo::getSimulationTime);
	classUpdateInfo->addMethod("getSimulationDeltaTime", &UpdateInfo::getSimulationDeltaTime);
	classUpdateInfo->addMethod("getSimulationFrequency", &UpdateInfo::getSimulationFrequency);
	classUpdateInfo->addMethod("getFrameDeltaTime", &UpdateInfo::getFrameDeltaTime);
	classUpdateInfo->addMethod("getInterval", &UpdateInfo::getInterval);
	classUpdateInfo->addMethod("isRunningSlow", &UpdateInfo::isRunningSlow);
	registrar->registerClass(classUpdateInfo);

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

	Ref< AutoRuntimeClass< SparkLayer > > classSparkLayer = new AutoRuntimeClass< SparkLayer >();
	classSparkLayer->addMethod("getRoot", &SparkLayer::getRoot);
	registrar->registerClass(classSparkLayer);

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
	classWorldLayer->addMethod("getEntity", &WorldLayer_getEntity_1);
	classWorldLayer->addMethod("getEntity", &WorldLayer_getEntity_2);
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
	classWorldLayer->addMethod("getImageProcess", &WorldLayer::getImageProcess);
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
}

	}
}
