/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/Game/GameClassFactory.h"
#include "Amalgam/Game/IAudioServer.h"
#include "Amalgam/Game/IEnvironment.h"
#include "Amalgam/Game/IInputServer.h"
#include "Amalgam/Game/IRenderServer.h"
#include "Amalgam/Game/UpdateControl.h"
#include "Amalgam/Game/UpdateInfo.h"
#include "Amalgam/Game/Engine/AudioLayer.h"
#include "Amalgam/Game/Engine/StageData.h"
#include "Amalgam/Game/Engine/FlashLayer.h"
#include "Amalgam/Game/Engine/Stage.h"
#include "Amalgam/Game/Engine/StageData.h"
#include "Amalgam/Game/Engine/StageLoader.h"
#include "Amalgam/Game/Engine/VideoLayer.h"
#include "Amalgam/Game/Engine/WorldLayer.h"
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Settings/PropertyGroup.h"
#include "Database/Database.h"
#include "Drawing/Image.h"
#include "Flash/Movie.h"
#include "Flash/MoviePlayer.h"
#include "Flash/SpriteInstance.h"
#include "Flash/Action/ActionContext.h"
#include "Input/InputSystem.h"
#include "Input/RumbleEffectPlayer.h"
#include "Input/Binding/InputMapping.h"
#include "Online/ISessionManager.h"
#include "Physics/PhysicsManager.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/ImageProcess/ImageProcess.h"
#include "Resource/IResourceManager.h"
#include "Sound/SoundSystem.h"
#include "Sound/Filters/SurroundEnvironment.h"
#include "Sound/Player/ISoundPlayer.h"
#include "World/Entity.h"
#include "World/EntityData.h"
#include "World/IEntityBuilder.h"
#include "World/IEntityEventManager.h"
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

bool IInputServer_fabricateInputSource(IInputServer* self, const std::wstring& sourceId, int32_t category, bool analogue)
{
	return self->fabricateInputSource(sourceId, (input::InputCategory)category, analogue);
}

RefArray< BoxedTransition > StageData_getTransitions(StageData* self)
{
	RefArray< BoxedTransition > out;

	const std::map< std::wstring, Guid >& transitions = self->getTransitions();
	for (std::map< std::wstring, Guid >::const_iterator i = transitions.begin(); i != transitions.end(); ++i)
		out.push_back(new BoxedTransition(i->first, i->second));

	return out;
}

Any FlashLayer_externalCall(FlashLayer* self, const std::string& methodName, uint32_t argc, const Any* argv)
{
	return self->externalCall(methodName, argc, argv);
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

Ref< BoxedRay3 > WorldLayer_screenToView(WorldLayer* self, const Vector2& screenPosition)
{
	Ray3 viewRay;
	if (self->screenToView(screenPosition, viewRay))
		return new BoxedRay3(viewRay);
	else
		return 0;
}

Ref< BoxedRay3 > WorldLayer_screenToWorld(WorldLayer* self, const Vector2& screenPosition)
{
	Ray3 worldRay;
	if (self->screenToWorld(screenPosition, worldRay))
		return new BoxedRay3(worldRay);
	else
		return 0;
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.GameClassFactory", 0, GameClassFactory, IRuntimeClassFactory)

void GameClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< IEnvironment > > classEnvironment = new AutoRuntimeClass< IEnvironment >();
	classEnvironment->addProperty< db::Database* >("database", &IEnvironment::getDatabase);
	classEnvironment->addProperty< UpdateControl* >("control", &IEnvironment::getControl);
	classEnvironment->addProperty< IAudioServer* >("audio", &IEnvironment::getAudio);
	classEnvironment->addProperty< IInputServer* >("input", &IEnvironment::getInput);
	classEnvironment->addProperty< IOnlineServer* >("online", &IEnvironment::getOnline);
	classEnvironment->addProperty< IPhysicsServer* >("physics", &IEnvironment::getPhysics);
	classEnvironment->addProperty< IRenderServer* >("render", &IEnvironment::getRender);
	classEnvironment->addProperty< IResourceServer* >("resource", &IEnvironment::getResource);
	classEnvironment->addProperty< IScriptServer* >("script", &IEnvironment::getScript);
	classEnvironment->addProperty< IWorldServer* >("world", &IEnvironment::getWorld);
	classEnvironment->addProperty< PropertyGroup* >("settings", &IEnvironment::getSettings);
	classEnvironment->addMethod("reconfigure", &IEnvironment::reconfigure);
	registrar->registerClass(classEnvironment);

	Ref< AutoRuntimeClass< IAudioServer > > classAudioServer = new AutoRuntimeClass< IAudioServer >();
	classAudioServer->addProperty< sound::SoundSystem* >("soundSystem", 0, &IAudioServer::getSoundSystem);
	classAudioServer->addProperty< sound::ISoundPlayer* >("soundPlayer", 0, &IAudioServer::getSoundPlayer);
	classAudioServer->addProperty< sound::SurroundEnvironment* >("surroundEnvironment", 0, &IAudioServer::getSurroundEnvironment);
	registrar->registerClass(classAudioServer);

	Ref< AutoRuntimeClass< IInputServer > > classInputServer = new AutoRuntimeClass< IInputServer >();
	classInputServer->addProperty< input::InputSystem* >("inputSystem", 0, &IInputServer::getInputSystem);
	classInputServer->addProperty< input::InputMapping* >("inputMapping", 0, &IInputServer::getInputMapping);
	classInputServer->addProperty< input::RumbleEffectPlayer* >("rumbleEffectPlayer", 0, &IInputServer::getRumbleEffectPlayer);
	classInputServer->addProperty< bool >("isFabricating", 0, &IInputServer::isFabricating);
	classInputServer->addProperty< bool >("isIdle", 0, &IInputServer::isIdle);
	classInputServer->addMethod("fabricateInputSource", &IInputServer_fabricateInputSource);
	classInputServer->addMethod("abortedFabricating", &IInputServer::abortedFabricating);
	classInputServer->addMethod("resetInputSource", &IInputServer::resetInputSource);
	classInputServer->addMethod("apply", &IInputServer::apply);
	classInputServer->addMethod("revert", &IInputServer::revert);
	registrar->registerClass(classInputServer);

	Ref< AutoRuntimeClass< IOnlineServer > > classOnlineServer = new AutoRuntimeClass< IOnlineServer >();
	classOnlineServer->addProperty< online::ISessionManager* >("sessionManager", 0, &IOnlineServer::getSessionManager);
	registrar->registerClass(classOnlineServer);

	Ref< AutoRuntimeClass< IPhysicsServer > > classPhysicsServer = new AutoRuntimeClass< IPhysicsServer >();
	classPhysicsServer->addProperty< physics::PhysicsManager* >("physicsManager", 0, &IPhysicsServer::getPhysicsManager);
	registrar->registerClass(classPhysicsServer);

	Ref< AutoRuntimeClass< IRenderServer > > classRenderServer = new AutoRuntimeClass< IRenderServer >();
	classRenderServer->addProperty< render::IRenderSystem* >("renderSystem", 0, &IRenderServer::getRenderSystem);
	classRenderServer->addProperty< render::IRenderView* >("renderView", 0, &IRenderServer::getRenderView);
	classRenderServer->addProperty< float >("screenAspectRatio", 0, &IRenderServer::getScreenAspectRatio);
	classRenderServer->addProperty< float >("viewAspectRatio", 0, &IRenderServer::getViewAspectRatio);
	classRenderServer->addProperty< float >("aspectRatio", 0, &IRenderServer::getAspectRatio);
	classRenderServer->addProperty< bool >("stereoscopic", 0, &IRenderServer::getStereoscopic);
	classRenderServer->addProperty< int32_t >("multiSample", 0, &IRenderServer::getMultiSample);
	classRenderServer->addProperty< int32_t >("frameRate", 0, &IRenderServer::getFrameRate);
	registrar->registerClass(classRenderServer);

	Ref< AutoRuntimeClass< IResourceServer > > classResourceServer = new AutoRuntimeClass< IResourceServer >();
	classResourceServer->addProperty< resource::IResourceManager* >("resourceManager", 0, &IResourceServer::getResourceManager);
	registrar->registerClass(classResourceServer);

	Ref< AutoRuntimeClass< IWorldServer > > classWorldServer = new AutoRuntimeClass< IWorldServer >();
	classWorldServer->addProperty< const world::IEntityBuilder* >("entityBuilder", 0, &IWorldServer::getEntityBuilder);
	classWorldServer->addProperty< world::WorldEntityRenderers* >("entityRenderers", 0, &IWorldServer::getEntityRenderers);
	classWorldServer->addProperty< world::IEntityEventManager* >("entityEventManager", 0, &IWorldServer::getEntityEventManager);
	classWorldServer->addMethod("addEntityFactory", &IWorldServer::addEntityFactory);
	classWorldServer->addMethod("removeEntityFactory", &IWorldServer::removeEntityFactory);
	classWorldServer->addMethod("addEntityRenderer", &IWorldServer::addEntityRenderer);
	classWorldServer->addMethod("removeEntityRenderer", &IWorldServer::removeEntityRenderer);
	registrar->registerClass(classWorldServer);

	Ref< AutoRuntimeClass< UpdateControl > > classUpdateControl = new AutoRuntimeClass< UpdateControl >();
	classUpdateControl->addProperty< bool >("pause", &UpdateControl::setPause, &UpdateControl::getPause);
	classUpdateControl->addProperty< float >("timeScale", &UpdateControl::setTimeScale, &UpdateControl::getTimeScale);
	classUpdateControl->addProperty< float >("simulationFrequency", &UpdateControl::setSimulationFrequency, &UpdateControl::getSimulationFrequency);
	registrar->registerClass(classUpdateControl);

	Ref< AutoRuntimeClass< UpdateInfo > > classUpdateInfo = new AutoRuntimeClass< UpdateInfo >();
	classUpdateInfo->addProperty< float >("totalTime", 0, &UpdateInfo::getTotalTime);
	classUpdateInfo->addProperty< float >("stateTime", 0, &UpdateInfo::getStateTime);
	classUpdateInfo->addProperty< float >("simulationTime", 0, &UpdateInfo::getSimulationTime);
	classUpdateInfo->addProperty< float >("simulationDeltaTime", 0, &UpdateInfo::getSimulationDeltaTime);
	classUpdateInfo->addProperty< int32_t >("simulationFrequency", 0, &UpdateInfo::getSimulationFrequency);
	classUpdateInfo->addProperty< float >("frameDeltaTime", 0, &UpdateInfo::getFrameDeltaTime);
	classUpdateInfo->addProperty< float >("interval", 0, &UpdateInfo::getInterval);
	classUpdateInfo->addProperty< bool >("isRunningSlow", 0, &UpdateInfo::isRunningSlow);
	registrar->registerClass(classUpdateInfo);

	Ref< AutoRuntimeClass< BoxedTransition > > classBoxedTransition = new AutoRuntimeClass< BoxedTransition >();
	classBoxedTransition->addProperty< const std::wstring& >("id", 0, &BoxedTransition::getId);
	classBoxedTransition->addProperty< const Guid& >("reference", 0, &BoxedTransition::getReference);
	registrar->registerClass(classBoxedTransition);

	Ref< AutoRuntimeClass< StageData > > classStageData = new AutoRuntimeClass< StageData >();
	classStageData->addProperty< const std::wstring& >("name", 0, &StageData::getName);
	classStageData->addProperty< const Guid& >("inherit", 0, &StageData::getInherit);
	classStageData->addProperty< float >("fadeRate", &StageData::setFadeRate, &StageData::getFadeRate);
	classStageData->addProperty< const Guid& >("resourceBundle", &StageData::setResourceBundle, &StageData::getResourceBundle);
	classStageData->addProperty< const PropertyGroup* >("properties", &StageData::setProperties, &StageData::getProperties);
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
	classLayer->addProperty< const std::wstring& >("name", 0, &Layer::getName);
	registrar->registerClass(classLayer);

	Ref< AutoRuntimeClass< AudioLayer > > classAudioLayer = new AutoRuntimeClass< AudioLayer >();
	classAudioLayer->addMethod("play", &AudioLayer::play);
	classAudioLayer->addMethod("stop", &AudioLayer::stop);
	classAudioLayer->addMethod("fadeOff", &AudioLayer::fadeOff);
	classAudioLayer->addMethod("setParameter", &AudioLayer::setParameter);
	classAudioLayer->addMethod("tweenParameter", &AudioLayer::tweenParameter);
	registrar->registerClass(classAudioLayer);

	Ref< AutoRuntimeClass< FlashLayer > > classFlashLayer = new AutoRuntimeClass< FlashLayer >();
	classFlashLayer->addProperty< flash::MoviePlayer* >("moviePlayer", 0, &FlashLayer::getMoviePlayer);
	classFlashLayer->addProperty< flash::ActionContext* >("context", 0, &FlashLayer::getContext);
	classFlashLayer->addProperty< flash::SpriteInstance* >("root", 0, &FlashLayer::getRoot);
	classFlashLayer->addProperty< bool >("visible", &FlashLayer::setVisible, &FlashLayer::isVisible);
	classFlashLayer->addMethod("getExternal", &FlashLayer::getExternal);
	classFlashLayer->addMethod("getPrintableString", &FlashLayer::getPrintableString);
	classFlashLayer->setUnknownHandler(&FlashLayer_externalCall);
	registrar->registerClass(classFlashLayer);

	Ref< AutoRuntimeClass< VideoLayer > > classVideoLayer = new AutoRuntimeClass< VideoLayer >();
	classVideoLayer->addProperty< bool >("playing", 0, &VideoLayer::isPlaying);
	classVideoLayer->addProperty< bool >("visible", 0, &VideoLayer::isVisible);
	classVideoLayer->addProperty< bool >("repeat", &VideoLayer::setRepeat, &VideoLayer::getRepeat);
	classVideoLayer->addMethod("play", &VideoLayer::play);
	classVideoLayer->addMethod("stop", &VideoLayer::stop);
	classVideoLayer->addMethod("rewind", &VideoLayer::rewind);
	classVideoLayer->addMethod("show", &VideoLayer::show);
	classVideoLayer->addMethod("hide", &VideoLayer::hide);
	classVideoLayer->addMethod("setScreenBounds", &VideoLayer::setScreenBounds);
	classVideoLayer->addMethod("getScreenBounds", &VideoLayer::getScreenBounds);
	registrar->registerClass(classVideoLayer);

	Ref< AutoRuntimeClass< WorldLayer > > classWorldLayer = new AutoRuntimeClass< WorldLayer >();
	classWorldLayer->addProperty< world::IEntitySchema* >("entitySchema", &WorldLayer::getEntitySchema);
	classWorldLayer->addProperty< render::ImageProcess* >("imageProcess", &WorldLayer::getImageProcess);
	classWorldLayer->addProperty< const Frustum& >("viewFrustum", &WorldLayer::getViewFrustum);
	classWorldLayer->addProperty< float >("fieldOfView", &WorldLayer::setFieldOfView, &WorldLayer::getFieldOfView);
	classWorldLayer->addProperty< float >("alternateTime", &WorldLayer::setAlternateTime, &WorldLayer::getAlternateTime);
	classWorldLayer->addProperty< float >("feedbackScale", &WorldLayer::setFeedbackScale, &WorldLayer::getFeedbackScale);
	classWorldLayer->addProperty< const world::Entity* >("camera", &WorldLayer::setCamera, &WorldLayer::getCamera);
	classWorldLayer->addProperty< const world::Entity* >("listener", &WorldLayer::setListener, &WorldLayer::getListener);
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
	classWorldLayer->addMethod("removeEntity", &WorldLayer::removeEntity);
	classWorldLayer->addMethod("isEntityAdded", &WorldLayer::isEntityAdded);
	classWorldLayer->addMethod("setControllerEnable", &WorldLayer::setControllerEnable);
	classWorldLayer->addMethod("resetController", &WorldLayer::resetController);
	classWorldLayer->addMethod("worldToView", &WorldLayer_worldToView);
	classWorldLayer->addMethod("viewToWorld", &WorldLayer_viewToWorld);
	classWorldLayer->addMethod("worldToScreen", &WorldLayer_worldToScreen);
	classWorldLayer->addMethod("viewToScreen", &WorldLayer_viewToScreen);
	classWorldLayer->addMethod("screenToView", &WorldLayer_screenToView);
	classWorldLayer->addMethod("screenToWorld", &WorldLayer_screenToWorld);
	registrar->registerClass(classWorldLayer);

	Ref< AutoRuntimeClass< StageLoader > > classStageLoader = new AutoRuntimeClass< StageLoader >();
	classStageLoader->addProperty< bool >("ready", 0, &StageLoader::ready);
	classStageLoader->addProperty< bool >("succeeded", 0, &StageLoader::succeeded);
	classStageLoader->addProperty< bool >("failed", 0, &StageLoader::failed);
	classStageLoader->addMethod("wait", &StageLoader::wait);
	classStageLoader->addMethod("get", &StageLoader::get);
	registrar->registerClass(classStageLoader);
}

	}
}
