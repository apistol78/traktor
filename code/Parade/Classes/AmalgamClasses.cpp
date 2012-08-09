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
#include "World/IWorldRenderer.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldRenderSettings.h"
#include "World/Entity/IEntityBuilder.h"
#include "World/Entity/IEntityFactory.h"
#include "World/Entity/IEntityRenderer.h"

namespace traktor
{
	namespace parade
	{

void registerAmalgamClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< amalgam::IEnvironment > > classEnvironment = new script::AutoScriptClass< amalgam::IEnvironment >();
	classEnvironment->addMethod(L"getDatabase", &amalgam::IEnvironment::getDatabase);
	classEnvironment->addMethod(L"getAudio", &amalgam::IEnvironment::getAudio);
	classEnvironment->addMethod(L"getInput", &amalgam::IEnvironment::getInput);
	classEnvironment->addMethod(L"getOnline", &amalgam::IEnvironment::getOnline);
	classEnvironment->addMethod(L"getPhysics", &amalgam::IEnvironment::getPhysics);
	classEnvironment->addMethod(L"getRender", &amalgam::IEnvironment::getRender);
	classEnvironment->addMethod(L"getResource", &amalgam::IEnvironment::getResource);
	classEnvironment->addMethod(L"getScript", &amalgam::IEnvironment::getScript);
	classEnvironment->addMethod(L"getWorld", &amalgam::IEnvironment::getWorld);
	classEnvironment->addMethod(L"getSettings", &amalgam::IEnvironment::getSettings);
	classEnvironment->addMethod(L"reconfigure", &amalgam::IEnvironment::reconfigure);
	scriptManager->registerClass(classEnvironment);

	Ref< script::AutoScriptClass< amalgam::IAudioServer > > classAudioServer = new script::AutoScriptClass< amalgam::IAudioServer >();
	classAudioServer->addMethod(L"getSoundSystem", &amalgam::IAudioServer::getSoundSystem);
	classAudioServer->addMethod(L"getSurroundEnvironment", &amalgam::IAudioServer::getSurroundEnvironment);
	scriptManager->registerClass(classAudioServer);

	Ref< script::AutoScriptClass< amalgam::IInputServer > > classInputServer = new script::AutoScriptClass< amalgam::IInputServer >();
	classInputServer->addMethod(L"isFabricating", &amalgam::IInputServer::isFabricating);
	classInputServer->addMethod(L"abortedFabricating", &amalgam::IInputServer::abortedFabricating);
	classInputServer->addMethod(L"resetInputSource", &amalgam::IInputServer::resetInputSource);
	classInputServer->addMethod(L"getInputSystem", &amalgam::IInputServer::getInputSystem);
	classInputServer->addMethod(L"getInputMapping", &amalgam::IInputServer::getInputMapping);
	classInputServer->addMethod(L"getRumbleEffectPlayer", &amalgam::IInputServer::getRumbleEffectPlayer);
	scriptManager->registerClass(classInputServer);

	Ref< script::AutoScriptClass< amalgam::IOnlineServer > > classOnlineServer = new script::AutoScriptClass< amalgam::IOnlineServer >();
	classOnlineServer->addMethod(L"getSessionManager", &amalgam::IOnlineServer::getSessionManager);
	scriptManager->registerClass(classOnlineServer);

	Ref< script::AutoScriptClass< amalgam::IPhysicsServer > > classPhysicsServer = new script::AutoScriptClass< amalgam::IPhysicsServer >();
	classPhysicsServer->addMethod(L"getPhysicsManager", &amalgam::IPhysicsServer::getPhysicsManager);
	scriptManager->registerClass(classPhysicsServer);

	Ref< script::AutoScriptClass< amalgam::IRenderServer > > classRenderServer = new script::AutoScriptClass< amalgam::IRenderServer >();
	classRenderServer->addMethod(L"getRenderSystem", &amalgam::IRenderServer::getRenderSystem);
	classRenderServer->addMethod(L"getRenderView", &amalgam::IRenderServer::getRenderView);
	classRenderServer->addMethod(L"getScreenAspectRatio", &amalgam::IRenderServer::getScreenAspectRatio);
	classRenderServer->addMethod(L"getViewAspectRatio", &amalgam::IRenderServer::getViewAspectRatio);
	classRenderServer->addMethod(L"getAspectRatio", &amalgam::IRenderServer::getAspectRatio);
	classRenderServer->addMethod(L"getStereoscopic", &amalgam::IRenderServer::getStereoscopic);
	classRenderServer->addMethod(L"getMultiSample", &amalgam::IRenderServer::getMultiSample);
	scriptManager->registerClass(classRenderServer);

	Ref< script::AutoScriptClass< amalgam::IResourceServer > > classResourceServer = new script::AutoScriptClass< amalgam::IResourceServer >();
	classResourceServer->addMethod(L"getResourceManager", &amalgam::IResourceServer::getResourceManager);
	scriptManager->registerClass(classResourceServer);

	Ref< script::AutoScriptClass< amalgam::IWorldServer > > classWorldServer = new script::AutoScriptClass< amalgam::IWorldServer >();
	classWorldServer->addMethod(L"addEntityFactory", &amalgam::IWorldServer::addEntityFactory);
	classWorldServer->addMethod(L"removeEntityFactory", &amalgam::IWorldServer::removeEntityFactory);
	classWorldServer->addMethod(L"addEntityRenderer", &amalgam::IWorldServer::addEntityRenderer);
	classWorldServer->addMethod(L"removeEntityRenderer", &amalgam::IWorldServer::removeEntityRenderer);
	classWorldServer->addMethod(L"getEntityBuilder", &amalgam::IWorldServer::getEntityBuilder);
	classWorldServer->addMethod(L"getEntityRenderers", &amalgam::IWorldServer::getEntityRenderers);
	classWorldServer->addMethod(L"createWorldRenderer", &amalgam::IWorldServer::createWorldRenderer);
	classWorldServer->addMethod(L"getFrameCount", &amalgam::IWorldServer::getFrameCount);
	scriptManager->registerClass(classWorldServer);

	Ref< script::AutoScriptClass< amalgam::IUpdateControl > > classIUpdateControl = new script::AutoScriptClass< amalgam::IUpdateControl >();
	classIUpdateControl->addMethod(L"setPause", &amalgam::IUpdateControl::setPause);
	classIUpdateControl->addMethod(L"getPause", &amalgam::IUpdateControl::getPause);
	scriptManager->registerClass(classIUpdateControl);

	Ref< script::AutoScriptClass< amalgam::IUpdateInfo > > classIUpdateInfo = new script::AutoScriptClass< amalgam::IUpdateInfo >();
	classIUpdateInfo->addMethod(L"getTotalTime", &amalgam::IUpdateInfo::getTotalTime);
	classIUpdateInfo->addMethod(L"getStateTime", &amalgam::IUpdateInfo::getStateTime);
	classIUpdateInfo->addMethod(L"getSimulationTime", &amalgam::IUpdateInfo::getSimulationTime);
	classIUpdateInfo->addMethod(L"getSimulationDeltaTime", &amalgam::IUpdateInfo::getSimulationDeltaTime);
	classIUpdateInfo->addMethod(L"getSimulationFrequency", &amalgam::IUpdateInfo::getSimulationFrequency);
	classIUpdateInfo->addMethod(L"getFrameDeltaTime", &amalgam::IUpdateInfo::getFrameDeltaTime);
	classIUpdateInfo->addMethod(L"getInterval", &amalgam::IUpdateInfo::getInterval);
	scriptManager->registerClass(classIUpdateInfo);
}

	}
}
