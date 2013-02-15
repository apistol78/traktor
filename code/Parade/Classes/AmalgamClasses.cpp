#include "Amalgam/IEnvironment.h"
#include "Amalgam/IInputServer.h"
#include "Amalgam/IRenderServer.h"
#include "Amalgam/IUpdateControl.h"
#include "Amalgam/IUpdateInfo.h"
#include "Input/InputSystem.h"
#include "Input/RumbleEffectPlayer.h"
#include "Input/Binding/InputMapping.h"
#include "Online/ISessionManager.h"
#include "Parade/Classes/AmalgamClasses.h"
#include "Physics/PhysicsManager.h"
#include "Resource/IResourceManager.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Script/AutoScriptClass.h"
#include "Script/IScriptManager.h"
#include "Sound/SoundSystem.h"
#include "Sound/Filters/SurroundEnvironment.h"
#include "World/IEntityBuilder.h"
#include "World/IEntityFactory.h"
#include "World/IEntityRenderer.h"
#include "World/IWorldRenderer.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldRenderSettings.h"

namespace traktor
{
	namespace parade
	{

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
}

	}
}
