#include "Amalgam/IEnvironment.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Amalgam/RuntimePlugin.h"
#include "Amalgam/Stage.h"
#include "Amalgam/StageData.h"
#include "Amalgam/StageState.h"
#include "Amalgam/Classes/AiClasses.h"
#include "Amalgam/Classes/AmalgamClasses.h"
#include "Amalgam/Classes/AnimationClasses.h"
#include "Amalgam/Classes/CoreClasses.h"
#include "Amalgam/Classes/DatabaseClasses.h"
#include "Amalgam/Classes/DrawingClasses.h"
#include "Amalgam/Classes/FlashClasses.h"
#include "Amalgam/Classes/I18NClasses.h"
#include "Amalgam/Classes/InputClasses.h"
#include "Amalgam/Classes/MeshClasses.h"
#include "Amalgam/Classes/NetClasses.h"
#include "Amalgam/Classes/OnlineClasses.h"
#include "Amalgam/Classes/PhysicsClasses.h"
#include "Amalgam/Classes/RenderClasses.h"
#include "Amalgam/Classes/ResourceClasses.h"
#include "Amalgam/Classes/SoundClasses.h"
#include "Amalgam/Classes/SprayClasses.h"
#include "Amalgam/Classes/TerrainClasses.h"
#include "Amalgam/Classes/WorldClasses.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.RuntimePlugin", 0, RuntimePlugin, amalgam::IRuntimePlugin)

bool RuntimePlugin::getDependencies(TypeInfoSet& outDependencies) const
{
	return true;
}

bool RuntimePlugin::startup(amalgam::IEnvironment* environment)
{
	script::IScriptManager* scriptManager = environment->getScript()->getScriptManager();
	T_ASSERT (scriptManager);

	registerCoreClasses(scriptManager);
	registerDrawingClasses(scriptManager);
	registerDatabaseClasses(scriptManager);
	registerResourceClasses(scriptManager);
	registerRenderClasses(scriptManager);
	registerWorldClasses(scriptManager);
	registerAmalgamClasses(scriptManager);
	registerAiClasses(scriptManager);
	registerFlashClasses(scriptManager);
	registerInputClasses(scriptManager);
	registerMeshClasses(scriptManager);
	registerPhysicsClasses(scriptManager);
	registerSoundClasses(scriptManager);
	registerAnimationClasses(scriptManager);
	registerNetClasses(scriptManager);
	registerOnlineClasses(scriptManager);
	registerTerrainClasses(scriptManager);
	registerSprayClasses(scriptManager);
	registerI18NClasses(scriptManager);

	return true;
}

bool RuntimePlugin::shutdown(amalgam::IEnvironment* environment)
{
	return true;
}

Ref< amalgam::IState > RuntimePlugin::createInitialState(amalgam::IEnvironment* environment) const
{
	Guid startupGuid(environment->getSettings()->getProperty< PropertyString >(L"Amalgam.Startup"));

	Ref< StageData > stageData = environment->getDatabase()->getObjectReadOnly< StageData >(startupGuid);
	if (!stageData)
	{
		log::error << L"Unable to read startup stage; startup failed" << Endl;
		return 0;
	}

	Ref< Stage > stage = stageData->createInstance(environment, 0);
	if (!stage)
	{
		log::error << L"Unable to create startup stage; startup failed" << Endl;
		return 0;
	}

	return new StageState(environment, stage);
}

	}
}
