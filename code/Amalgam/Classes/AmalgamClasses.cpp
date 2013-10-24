#include "Amalgam/AudioLayer.h"
#include "Amalgam/IEnvironment.h"
#include "Amalgam/IInputServer.h"
#include "Amalgam/IRenderServer.h"
#include "Amalgam/IUpdateControl.h"
#include "Amalgam/IUpdateInfo.h"
#include "Amalgam/Stage.h"
#include "Amalgam/StageData.h"
#include "Amalgam/StageLoader.h"
#include "Amalgam/VideoLayer.h"
#include "Amalgam/WorldLayer.h"
#include "Amalgam/Classes/AmalgamClasses.h"
#include "Input/InputSystem.h"
#include "Input/RumbleEffectPlayer.h"
#include "Input/Binding/InputMapping.h"
#include "Online/ISessionManager.h"
#include "Physics/PhysicsManager.h"
#include "Resource/IResourceManager.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Script/AutoScriptClass.h"
#include "Script/Boxes.h"
#include "Script/IScriptManager.h"
#include "Sound/SoundSystem.h"
#include "Sound/Filters/SurroundEnvironment.h"
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

Ref< world::Entity > WorldLayer_createEntity1(WorldLayer* self, const std::wstring& name, world::IEntitySchema* entitySchema)
{
	return self->createEntity(name, entitySchema);
}

Ref< world::Entity > WorldLayer_createEntity2(WorldLayer* self, const std::wstring& name)
{
	return self->createEntity(name, 0);
}

Ref< script::BoxedVector4 > WorldLayer_worldToView(WorldLayer* self, const Vector4& worldPosition)
{
	Vector4 viewPosition;
	if (self->worldToView(worldPosition, viewPosition))
		return new script::BoxedVector4(viewPosition);
	else
		return 0;
}

Ref< script::BoxedVector4 > WorldLayer_viewToWorld(WorldLayer* self, const Vector4& viewPosition)
{
	Vector4 worldPosition;
	if (self->viewToWorld(viewPosition, worldPosition))
		return new script::BoxedVector4(worldPosition);
	else
		return 0;
}

Ref< script::BoxedVector2 > WorldLayer_worldToScreen(WorldLayer* self, const Vector4& worldPosition)
{
	Vector2 screenPosition;
	if (self->worldToScreen(worldPosition, screenPosition))
		return new script::BoxedVector2(screenPosition);
	else
		return 0;
}

Ref< script::BoxedVector2 > WorldLayer_viewToScreen(WorldLayer* self, const Vector4& viewPosition)
{
	Vector2 screenPosition;
	if (self->viewToScreen(viewPosition, screenPosition))
		return new script::BoxedVector2(screenPosition);
	else
		return 0;
}

	}

void registerAmalgamClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< amalgam::IEnvironment > > classEnvironment = new script::AutoScriptClass< amalgam::IEnvironment >();
	classEnvironment->addMethod("getDatabase", &amalgam::IEnvironment::getDatabase);
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
	scriptManager->registerClass(classEnvironment);

	Ref< script::AutoScriptClass< amalgam::IAudioServer > > classAudioServer = new script::AutoScriptClass< amalgam::IAudioServer >();
	classAudioServer->addMethod("getSoundSystem", &amalgam::IAudioServer::getSoundSystem);
	classAudioServer->addMethod("getSurroundEnvironment", &amalgam::IAudioServer::getSurroundEnvironment);
	scriptManager->registerClass(classAudioServer);

	Ref< script::AutoScriptClass< amalgam::IInputServer > > classInputServer = new script::AutoScriptClass< amalgam::IInputServer >();
	classInputServer->addMethod("isFabricating", &amalgam::IInputServer::isFabricating);
	classInputServer->addMethod("abortedFabricating", &amalgam::IInputServer::abortedFabricating);
	classInputServer->addMethod("resetInputSource", &amalgam::IInputServer::resetInputSource);
	classInputServer->addMethod("isIdle", &amalgam::IInputServer::isIdle);
	classInputServer->addMethod("getInputSystem", &amalgam::IInputServer::getInputSystem);
	classInputServer->addMethod("getInputMapping", &amalgam::IInputServer::getInputMapping);
	classInputServer->addMethod("getRumbleEffectPlayer", &amalgam::IInputServer::getRumbleEffectPlayer);
	scriptManager->registerClass(classInputServer);

	Ref< script::AutoScriptClass< amalgam::IOnlineServer > > classOnlineServer = new script::AutoScriptClass< amalgam::IOnlineServer >();
	classOnlineServer->addMethod("getSessionManager", &amalgam::IOnlineServer::getSessionManager);
	scriptManager->registerClass(classOnlineServer);

	Ref< script::AutoScriptClass< amalgam::IPhysicsServer > > classPhysicsServer = new script::AutoScriptClass< amalgam::IPhysicsServer >();
	classPhysicsServer->addMethod("getPhysicsManager", &amalgam::IPhysicsServer::getPhysicsManager);
	scriptManager->registerClass(classPhysicsServer);

	Ref< script::AutoScriptClass< amalgam::IRenderServer > > classRenderServer = new script::AutoScriptClass< amalgam::IRenderServer >();
	classRenderServer->addMethod("getRenderSystem", &amalgam::IRenderServer::getRenderSystem);
	classRenderServer->addMethod("getRenderView", &amalgam::IRenderServer::getRenderView);
	classRenderServer->addMethod("getScreenAspectRatio", &amalgam::IRenderServer::getScreenAspectRatio);
	classRenderServer->addMethod("getViewAspectRatio", &amalgam::IRenderServer::getViewAspectRatio);
	classRenderServer->addMethod("getAspectRatio", &amalgam::IRenderServer::getAspectRatio);
	classRenderServer->addMethod("getStereoscopic", &amalgam::IRenderServer::getStereoscopic);
	classRenderServer->addMethod("getMultiSample", &amalgam::IRenderServer::getMultiSample);
	classRenderServer->addMethod("getFrameRate", &amalgam::IRenderServer::getFrameRate);
	scriptManager->registerClass(classRenderServer);

	Ref< script::AutoScriptClass< amalgam::IResourceServer > > classResourceServer = new script::AutoScriptClass< amalgam::IResourceServer >();
	classResourceServer->addMethod("getResourceManager", &amalgam::IResourceServer::getResourceManager);
	scriptManager->registerClass(classResourceServer);

	Ref< script::AutoScriptClass< amalgam::IWorldServer > > classWorldServer = new script::AutoScriptClass< amalgam::IWorldServer >();
	classWorldServer->addMethod("addEntityFactory", &amalgam::IWorldServer::addEntityFactory);
	classWorldServer->addMethod("removeEntityFactory", &amalgam::IWorldServer::removeEntityFactory);
	classWorldServer->addMethod("addEntityRenderer", &amalgam::IWorldServer::addEntityRenderer);
	classWorldServer->addMethod("removeEntityRenderer", &amalgam::IWorldServer::removeEntityRenderer);
	classWorldServer->addMethod("getEntityBuilder", &amalgam::IWorldServer::getEntityBuilder);
	classWorldServer->addMethod("getEntityRenderers", &amalgam::IWorldServer::getEntityRenderers);
	classWorldServer->addMethod("getEntityEventManager", &amalgam::IWorldServer::getEntityEventManager);
	classWorldServer->addMethod("getFrameCount", &amalgam::IWorldServer::getFrameCount);
	scriptManager->registerClass(classWorldServer);

	Ref< script::AutoScriptClass< amalgam::IUpdateControl > > classIUpdateControl = new script::AutoScriptClass< amalgam::IUpdateControl >();
	classIUpdateControl->addMethod("setPause", &amalgam::IUpdateControl::setPause);
	classIUpdateControl->addMethod("getPause", &amalgam::IUpdateControl::getPause);
	scriptManager->registerClass(classIUpdateControl);

	Ref< script::AutoScriptClass< amalgam::IUpdateInfo > > classIUpdateInfo = new script::AutoScriptClass< amalgam::IUpdateInfo >();
	classIUpdateInfo->addMethod("getTotalTime", &amalgam::IUpdateInfo::getTotalTime);
	classIUpdateInfo->addMethod("getStateTime", &amalgam::IUpdateInfo::getStateTime);
	classIUpdateInfo->addMethod("getSimulationTime", &amalgam::IUpdateInfo::getSimulationTime);
	classIUpdateInfo->addMethod("getSimulationDeltaTime", &amalgam::IUpdateInfo::getSimulationDeltaTime);
	classIUpdateInfo->addMethod("getSimulationFrequency", &amalgam::IUpdateInfo::getSimulationFrequency);
	classIUpdateInfo->addMethod("getFrameDeltaTime", &amalgam::IUpdateInfo::getFrameDeltaTime);
	classIUpdateInfo->addMethod("getInterval", &amalgam::IUpdateInfo::getInterval);
	scriptManager->registerClass(classIUpdateInfo);

	Ref< script::AutoScriptClass< BoxedTransition > > classBoxedTransition = new script::AutoScriptClass< BoxedTransition >();
	classBoxedTransition->addMethod("getId", &BoxedTransition::getId);
	classBoxedTransition->addMethod("getReference", &BoxedTransition::getReference);
	scriptManager->registerClass(classBoxedTransition);

	Ref< script::AutoScriptClass< StageData > > classStageData = new script::AutoScriptClass< StageData >();
	classStageData->addMethod("getTransitions", &StageData_getTransitions);
	scriptManager->registerClass(classStageData);

	Ref< script::AutoScriptClass< Stage > > classStage = new script::AutoScriptClass< Stage >();
	classStage->addMethod("addLayer", &Stage::addLayer);
	classStage->addMethod("removeLayer", &Stage::removeLayer);
	classStage->addMethod("removeAllLayers", &Stage::removeAllLayers);
	classStage->addMethod("findLayer", &Stage::findLayer);
	classStage->addMethod("terminate", &Stage::terminate);
	classStage->addMethod("loadStage", &Stage::loadStage);
	classStage->addMethod("loadStageAsync", &Stage::loadStageAsync);
	classStage->addMethod("gotoStage", &Stage::gotoStage);
	classStage->addMethod("getEnvironment", &Stage::getEnvironment);
	classStage->addMethod("getLayers", &Stage::getLayers);
	scriptManager->registerClass(classStage);

	Ref< script::AutoScriptClass< Layer > > classLayer = new script::AutoScriptClass< Layer >();
	classLayer->addMethod("getName", &Layer::getName);
	scriptManager->registerClass(classLayer);

	Ref< script::AutoScriptClass< AudioLayer > > classAudioLayer = new script::AutoScriptClass< AudioLayer >();
	classAudioLayer->addMethod("play", &AudioLayer::play);
	classAudioLayer->addMethod("stop", &AudioLayer::stop);
	classAudioLayer->addMethod("fadeOff", &AudioLayer::fadeOff);
	classAudioLayer->addMethod("setParameter", &AudioLayer::setParameter);
	classAudioLayer->addMethod("tweenParameter", &AudioLayer::tweenParameter);
	scriptManager->registerClass(classAudioLayer);

	Ref< script::AutoScriptClass< VideoLayer > > classVideoLayer = new script::AutoScriptClass< VideoLayer >();
	classVideoLayer->addMethod("playing", &VideoLayer::playing);
	scriptManager->registerClass(classVideoLayer);

	Ref< script::AutoScriptClass< WorldLayer > > classWorldLayer = new script::AutoScriptClass< WorldLayer >();
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
	classWorldLayer->addMethod("getEntitySchema", &WorldLayer::getEntitySchema);
	classWorldLayer->addMethod("setControllerEnable", &WorldLayer::setControllerEnable);
	classWorldLayer->addMethod("resetController", &WorldLayer::resetController);
	classWorldLayer->addMethod("getPostProcess", &WorldLayer::getPostProcess);
	classWorldLayer->addMethod("worldToView", &WorldLayer_worldToView);
	classWorldLayer->addMethod("viewToWorld", &WorldLayer_viewToWorld);
	classWorldLayer->addMethod("worldToScreen", &WorldLayer_worldToScreen);
	classWorldLayer->addMethod("viewToScreen", &WorldLayer_viewToScreen);
	classWorldLayer->addMethod("setFieldOfView", &WorldLayer::setFieldOfView);
	classWorldLayer->addMethod("getFieldOfView", &WorldLayer::getFieldOfView);
	classWorldLayer->addMethod("setAlternateTime", &WorldLayer::setAlternateTime);
	classWorldLayer->addMethod("getAlternateTime", &WorldLayer::getAlternateTime);
	scriptManager->registerClass(classWorldLayer);

	Ref< script::AutoScriptClass< StageLoader > > classStageLoader = new script::AutoScriptClass< StageLoader >();
	classStageLoader->addMethod("wait", &StageLoader::wait);
	classStageLoader->addMethod("ready", &StageLoader::ready);
	classStageLoader->addMethod("succeeded", &StageLoader::succeeded);
	classStageLoader->addMethod("failed", &StageLoader::failed);
	classStageLoader->addMethod("get", &StageLoader::get);
	scriptManager->registerClass(classStageLoader);
}

	}
}
