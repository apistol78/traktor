/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Class/IRuntimeDelegate.h"
#include "Core/Class/Boxes/BoxedAabb2.h"
#include "Core/Class/Boxes/BoxedColor4f.h"
#include "Core/Class/Boxes/BoxedFrustum.h"
#include "Core/Class/Boxes/BoxedGuid.h"
#include "Core/Class/Boxes/BoxedRay3.h"
#include "Core/Class/Boxes/BoxedRefArray.h"
#include "Core/Class/Boxes/BoxedTypeInfo.h"
#include "Core/Class/Boxes/BoxedVector2.h"
#include "Core/Settings/PropertyGroup.h"
#include "Database/Database.h"
#include "Drawing/Image.h"
#include "Input/InputSystem.h"
#include "Input/RumbleEffectPlayer.h"
#include "Input/Binding/InputMapping.h"
#include "Online/ISessionManager.h"
#include "Physics/PhysicsManager.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderClassFactory.h"
#include "Resource/IResourceManager.h"
#include "Runtime/GameClassFactory.h"
#include "Runtime/IAudioServer.h"
#include "Runtime/IEnvironment.h"
#include "Runtime/IInputServer.h"
#include "Runtime/IRenderServer.h"
#include "Runtime/UpdateControl.h"
#include "Runtime/UpdateInfo.h"
#include "Runtime/Engine/AudioLayer.h"
#include "Runtime/Engine/DebugLayer.h"
#include "Runtime/Engine/ScreenLayer.h"
#include "Runtime/Engine/SplitWorldLayer.h"
#include "Runtime/Engine/Stage.h"
#include "Runtime/Engine/StageData.h"
#include "Runtime/Engine/StageLoader.h"
#include "Runtime/Engine/VideoLayer.h"
#include "Runtime/Engine/WorldLayer.h"
#include "Scene/Scene.h"
#include "Sound/AudioSystem.h"
#include "Sound/Filters/SurroundEnvironment.h"
#include "Sound/Player/ISoundPlayer.h"
#include "World/Entity.h"
#include "World/EntityData.h"
#include "World/IEntityBuilder.h"
#include "World/IEntityFactory.h"
#include "World/IEntityRenderer.h"
#include "World/IWorldRenderer.h"
#include "World/World.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldRenderSettings.h"

namespace traktor::runtime
{
	namespace
	{

class BoxedTransition : public Object
{
	T_RTTI_CLASS;

public:
	explicit BoxedTransition(const std::wstring& id, const Guid& reference)
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.Transition", BoxedTransition, Object)

bool IInputServer_fabricateInputSource(IInputServer* self, const std::wstring& sourceId, int32_t category, bool analogue)
{
	return self->fabricateInputSource(sourceId, (input::InputCategory)category, analogue);
}

RefArray< BoxedTransition > StageData_getTransitions(StageData* self)
{
	RefArray< BoxedTransition > out;
	for (auto it : self->getTransitions())
		out.push_back(new BoxedTransition(it.first, it.second));
	return out;
}

void AudioLayer_tweenParameter_1(AudioLayer* self, const std::wstring& parameterName, float toValue, float duration)
{
	self->tweenParameter(parameterName, toValue, duration);
}

void AudioLayer_tweenParameter_2(AudioLayer* self, const std::wstring& parameterName, float fromValue, float toValue, float duration)
{
	self->tweenParameter(parameterName, fromValue, toValue, duration);
}

void ScreenLayer_setParameterCallback(ScreenLayer* self, IRuntimeDelegate* callback)
{
	if (callback)
	{
		Ref< IRuntimeDelegate > rc = callback;
		self->setParameterCallback([&, rc](render::ProgramParameters* programParameters) {
			Ref< render::BoxedProgramParameters > b = new render::BoxedProgramParameters(programParameters);
			const Any argv[] = { CastAny< render::BoxedProgramParameters* >::set(b) };
			rc->call(sizeof_array(argv), argv);
		 });
	}
	else
		self->setParameterCallback(nullptr);
}

Ref< BoxedVector4 > WorldLayer_worldToView(WorldLayer* self, const Vector4& worldPosition)
{
	Vector4 viewPosition;
	if (self->worldToView(worldPosition, viewPosition))
		return new BoxedVector4(viewPosition);
	else
		return nullptr;
}

Ref< BoxedVector4 > WorldLayer_viewToWorld(WorldLayer* self, const Vector4& viewPosition)
{
	Vector4 worldPosition;
	if (self->viewToWorld(viewPosition, worldPosition))
		return new BoxedVector4(worldPosition);
	else
		return nullptr;
}

Ref< BoxedVector2 > WorldLayer_worldToScreen(WorldLayer* self, const Vector4& worldPosition)
{
	Vector2 screenPosition;
	if (self->worldToScreen(worldPosition, screenPosition))
		return new BoxedVector2(screenPosition);
	else
		return nullptr;
}

Ref< BoxedVector2 > WorldLayer_viewToScreen(WorldLayer* self, const Vector4& viewPosition)
{
	Vector2 screenPosition;
	if (self->viewToScreen(viewPosition, screenPosition))
		return new BoxedVector2(screenPosition);
	else
		return nullptr;
}

Ref< BoxedRay3 > WorldLayer_screenToView(WorldLayer* self, const Vector2& screenPosition)
{
	Ray3 viewRay;
	if (self->screenToView(screenPosition, viewRay))
		return new BoxedRay3(viewRay);
	else
		return nullptr;
}

Ref< BoxedRay3 > WorldLayer_screenToWorld(WorldLayer* self, const Vector2& screenPosition)
{
	Ray3 worldRay;
	if (self->screenToWorld(screenPosition, worldRay))
		return new BoxedRay3(worldRay);
	else
		return nullptr;
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.GameClassFactory", 0, GameClassFactory, IRuntimeClassFactory)

void GameClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classEnvironment = new AutoRuntimeClass< IEnvironment >();
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

	auto classAudioServer = new AutoRuntimeClass< IAudioServer >();
	classAudioServer->addProperty< sound::AudioSystem* >("audioSystem", 0, &IAudioServer::getAudioSystem);
	classAudioServer->addProperty< sound::ISoundPlayer* >("soundPlayer", 0, &IAudioServer::getSoundPlayer);
	classAudioServer->addProperty< sound::SurroundEnvironment* >("surroundEnvironment", 0, &IAudioServer::getSurroundEnvironment);
	registrar->registerClass(classAudioServer);

	auto classInputServer = new AutoRuntimeClass< IInputServer >();
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

	auto classOnlineServer = new AutoRuntimeClass< IOnlineServer >();
	classOnlineServer->addProperty< online::ISessionManager* >("sessionManager", 0, &IOnlineServer::getSessionManager);
	registrar->registerClass(classOnlineServer);

	auto classPhysicsServer = new AutoRuntimeClass< IPhysicsServer >();
	classPhysicsServer->addProperty< physics::PhysicsManager* >("physicsManager", 0, &IPhysicsServer::getPhysicsManager);
	registrar->registerClass(classPhysicsServer);

	auto classRenderServer = new AutoRuntimeClass< IRenderServer >();
	classRenderServer->addProperty< render::IRenderSystem* >("renderSystem", 0, &IRenderServer::getRenderSystem);
	classRenderServer->addProperty< render::IRenderView* >("renderView", 0, &IRenderServer::getRenderView);
	classRenderServer->addProperty< float >("screenAspectRatio", 0, &IRenderServer::getScreenAspectRatio);
	classRenderServer->addProperty< float >("viewAspectRatio", 0, &IRenderServer::getViewAspectRatio);
	classRenderServer->addProperty< float >("aspectRatio", 0, &IRenderServer::getAspectRatio);
	classRenderServer->addProperty< int32_t >("multiSample", 0, &IRenderServer::getMultiSample);
	classRenderServer->addProperty< int32_t >("cpuDuration", 0, &IRenderServer::getCPUDuration);
	classRenderServer->addProperty< int32_t >("gpuDuration", 0, &IRenderServer::getGPUDuration);
	classRenderServer->addProperty< int32_t >("frameRate", 0, &IRenderServer::getFrameRate);
	registrar->registerClass(classRenderServer);

	auto classResourceServer = new AutoRuntimeClass< IResourceServer >();
	classResourceServer->addProperty< resource::IResourceManager* >("resourceManager", 0, &IResourceServer::getResourceManager);
	registrar->registerClass(classResourceServer);

	auto classWorldServer = new AutoRuntimeClass< IWorldServer >();
	classWorldServer->addProperty< const world::IEntityFactory* >("entityFactory", 0, &IWorldServer::getEntityFactory);
	classWorldServer->addProperty< world::WorldEntityRenderers* >("entityRenderers", 0, &IWorldServer::getEntityRenderers);
	classWorldServer->addMethod("addEntityFactory", &IWorldServer::addEntityFactory);
	classWorldServer->addMethod("removeEntityFactory", &IWorldServer::removeEntityFactory);
	classWorldServer->addMethod("addEntityRenderer", &IWorldServer::addEntityRenderer);
	classWorldServer->addMethod("removeEntityRenderer", &IWorldServer::removeEntityRenderer);
	registrar->registerClass(classWorldServer);

	auto classUpdateControl = new AutoRuntimeClass< UpdateControl >();
	classUpdateControl->addProperty< bool >("pause", &UpdateControl::setPause, &UpdateControl::getPause);
	classUpdateControl->addProperty< double >("timeScale", &UpdateControl::setTimeScale, &UpdateControl::getTimeScale);
	classUpdateControl->addProperty< double >("simulationFrequency", &UpdateControl::setSimulationFrequency, &UpdateControl::getSimulationFrequency);
	registrar->registerClass(classUpdateControl);

	auto classUpdateInfo = new AutoRuntimeClass< UpdateInfo >();
	classUpdateInfo->addProperty< double >("totalTime", 0, &UpdateInfo::getTotalTime);
	classUpdateInfo->addProperty< double >("stateTime", 0, &UpdateInfo::getStateTime);
	classUpdateInfo->addProperty< double >("simulationTime", 0, &UpdateInfo::getSimulationTime);
	classUpdateInfo->addProperty< double >("simulationDeltaTime", 0, &UpdateInfo::getSimulationDeltaTime);
	classUpdateInfo->addProperty< int32_t >("simulationFrequency", 0, &UpdateInfo::getSimulationFrequency);
	classUpdateInfo->addProperty< double >("frameDeltaTime", 0, &UpdateInfo::getFrameDeltaTime);
	classUpdateInfo->addProperty< float >("interval", 0, &UpdateInfo::getInterval);
	registrar->registerClass(classUpdateInfo);

	auto classBoxedTransition = new AutoRuntimeClass< BoxedTransition >();
	classBoxedTransition->addProperty< const std::wstring& >("id", 0, &BoxedTransition::getId);
	classBoxedTransition->addProperty< const Guid& >("reference", 0, &BoxedTransition::getReference);
	registrar->registerClass(classBoxedTransition);

	auto classStageData = new AutoRuntimeClass< StageData >();
	classStageData->addProperty< const std::wstring& >("name", 0, &StageData::getName);
	classStageData->addProperty< const Guid& >("inherit", 0, &StageData::getInherit);
	classStageData->addProperty< float >("fadeRate", &StageData::setFadeRate, &StageData::getFadeRate);
	classStageData->addProperty< const Guid& >("resourceBundle", &StageData::setResourceBundle, &StageData::getResourceBundle);
	classStageData->addProperty< const PropertyGroup* >("properties", &StageData::setProperties, &StageData::getProperties);
	classStageData->addMethod("getTransitions", &StageData_getTransitions);
	registrar->registerClass(classStageData);

	auto classStage = new AutoRuntimeClass< Stage >();
	classStage->addMethod("addLayer", &Stage::addLayer);
	classStage->addMethod("removeLayer", &Stage::removeLayer);
	classStage->addMethod("removeAllLayers", &Stage::removeAllLayers);
	classStage->addMethod("getLayer", &Stage::getLayer);
	classStage->addMethod("terminate", &Stage::terminate);
	classStage->addMethod("haveTransition", &Stage::haveTransition);
	classStage->addMethod("loadStage", &Stage::loadStage);
	classStage->addMethod("loadStageAsync", &Stage::loadStageAsync);
	classStage->addMethod("gotoStage", &Stage::gotoStage);
	classStage->addMethod("getEnvironment", &Stage::getEnvironment);
	classStage->addMethod("getLayers", &Stage::getLayers);
	registrar->registerClass(classStage);

	auto classLayer = new AutoRuntimeClass< Layer >();
	classLayer->addProperty< const std::wstring& >("name", 0, &Layer::getName);
	registrar->registerClass(classLayer);

	auto classAudioLayer = new AutoRuntimeClass< AudioLayer >();
	classAudioLayer->addMethod("play", &AudioLayer::play);
	classAudioLayer->addMethod("stop", &AudioLayer::stop);
	classAudioLayer->addMethod("fadeOff", &AudioLayer::fadeOff);
	classAudioLayer->addMethod("setParameter", &AudioLayer::setParameter);
	classAudioLayer->addMethod("tweenParameter", &AudioLayer_tweenParameter_1);
	classAudioLayer->addMethod("tweenParameter", &AudioLayer_tweenParameter_2);
	registrar->registerClass(classAudioLayer);

	auto classDebugLayer = new AutoRuntimeClass< DebugLayer >();
	classDebugLayer->addMethod("point", &DebugLayer::point);
	classDebugLayer->addMethod("line", &DebugLayer::line);
	registrar->registerClass(classDebugLayer);

	auto classScreenLayer = new AutoRuntimeClass< ScreenLayer >();
	classScreenLayer->addMethod("setParameterCallback", &ScreenLayer_setParameterCallback);
	registrar->registerClass(classScreenLayer);

	auto classSplitWorldLayer = new AutoRuntimeClass< SplitWorldLayer >();
	classSplitWorldLayer->addProperty("scene", &SplitWorldLayer::getScene);
	classSplitWorldLayer->addProperty("world", &SplitWorldLayer::getWorld);
	classSplitWorldLayer->addProperty("worldRenderer", &SplitWorldLayer::getWorldRenderer);
	// classSplitWorldLayer->addProperty< const Frustum& >("viewFrustum", &SplitWorldLayer::getViewFrustum);
	classSplitWorldLayer->addProperty< float >("fieldOfView", &SplitWorldLayer::setFieldOfView, &SplitWorldLayer::getFieldOfView);
	classSplitWorldLayer->addProperty< double >("alternateTime", &SplitWorldLayer::setAlternateTime, &SplitWorldLayer::getAlternateTime);
	// classSplitWorldLayer->addProperty< const world::Entity* >("camera", &SplitWorldLayer::setCamera, &SplitWorldLayer::getCamera);
	classSplitWorldLayer->addMethod("createEntity", &SplitWorldLayer::createEntity);
	classSplitWorldLayer->addMethod("setControllerEnable", &SplitWorldLayer::setControllerEnable);
	classSplitWorldLayer->addMethod("resetController", &SplitWorldLayer::resetController);
	// classSplitWorldLayer->addMethod("worldToView", &SplitWorldLayer_worldToView);
	// classSplitWorldLayer->addMethod("viewToWorld", &SplitWorldLayer_viewToWorld);
	// classSplitWorldLayer->addMethod("worldToScreen", &SplitWorldLayer_worldToScreen);
	// classSplitWorldLayer->addMethod("viewToScreen", &SplitWorldLayer_viewToScreen);
	// classSplitWorldLayer->addMethod("screenToView", &SplitWorldLayer_screenToView);
	// classSplitWorldLayer->addMethod("screenToWorld", &SplitWorldLayer_screenToWorld);
	registrar->registerClass(classSplitWorldLayer);

	auto classVideoLayer = new AutoRuntimeClass< VideoLayer >();
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

	auto classWorldLayer = new AutoRuntimeClass< WorldLayer >();
	classWorldLayer->addProperty("scene", &WorldLayer::getScene);
	classWorldLayer->addProperty("world", &WorldLayer::getWorld);
	classWorldLayer->addProperty("worldRenderer", &WorldLayer::getWorldRenderer);
	classWorldLayer->addProperty< const Frustum& >("viewFrustum", &WorldLayer::getViewFrustum);
	classWorldLayer->addProperty< float >("fieldOfView", &WorldLayer::setFieldOfView, &WorldLayer::getFieldOfView);
	classWorldLayer->addProperty< double >("alternateTime", &WorldLayer::setAlternateTime, &WorldLayer::getAlternateTime);
	classWorldLayer->addProperty< const world::Entity* >("camera", &WorldLayer::setCamera, &WorldLayer::getCamera);
	classWorldLayer->addMethod("createEntity", &WorldLayer::createEntity);
	classWorldLayer->addMethod("setControllerEnable", &WorldLayer::setControllerEnable);
	classWorldLayer->addMethod("resetController", &WorldLayer::resetController);
	classWorldLayer->addMethod("worldToView", &WorldLayer_worldToView);
	classWorldLayer->addMethod("viewToWorld", &WorldLayer_viewToWorld);
	classWorldLayer->addMethod("worldToScreen", &WorldLayer_worldToScreen);
	classWorldLayer->addMethod("viewToScreen", &WorldLayer_viewToScreen);
	classWorldLayer->addMethod("screenToView", &WorldLayer_screenToView);
	classWorldLayer->addMethod("screenToWorld", &WorldLayer_screenToWorld);
	registrar->registerClass(classWorldLayer);

	auto classStageLoader = new AutoRuntimeClass< StageLoader >();
	classStageLoader->addProperty< bool >("ready", 0, &StageLoader::ready);
	classStageLoader->addProperty< bool >("succeeded", 0, &StageLoader::succeeded);
	classStageLoader->addProperty< bool >("failed", 0, &StageLoader::failed);
	classStageLoader->addMethod("wait", &StageLoader::wait);
	classStageLoader->addMethod("get", &StageLoader::get);
	registrar->registerClass(classStageLoader);
}

}
