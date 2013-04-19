#include "Amalgam/IEnvironment.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Parade/RuntimePlugin.h"
#include "Parade/Stage.h"
#include "Parade/StageData.h"
#include "Parade/StageState.h"
#include "Parade/Classes/AmalgamClasses.h"
#include "Parade/Classes/AnimationClasses.h"
#include "Parade/Classes/CoreClasses.h"
#include "Parade/Classes/DatabaseClasses.h"
#include "Parade/Classes/DrawingClasses.h"
#include "Parade/Classes/FlashClasses.h"
#include "Parade/Classes/InputClasses.h"
#include "Parade/Classes/MeshClasses.h"
#include "Parade/Classes/NetClasses.h"
#include "Parade/Classes/OnlineClasses.h"
#include "Parade/Classes/ParadeClasses.h"
#include "Parade/Classes/PhysicsClasses.h"
#include "Parade/Classes/RenderClasses.h"
#include "Parade/Classes/ResourceClasses.h"
#include "Parade/Classes/SoundClasses.h"
#include "Parade/Classes/SprayClasses.h"
#include "Parade/Classes/TerrainClasses.h"
#include "Parade/Classes/WorldClasses.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.parade.RuntimePlugin", 0, RuntimePlugin, amalgam::IRuntimePlugin)

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
	registerFlashClasses(scriptManager);
	registerInputClasses(scriptManager);
	registerMeshClasses(scriptManager);
	registerParadeClasses(scriptManager);
	registerPhysicsClasses(scriptManager);
	registerSoundClasses(scriptManager);
	registerAnimationClasses(scriptManager);
	registerNetClasses(scriptManager);
	registerOnlineClasses(scriptManager);
	registerTerrainClasses(scriptManager);
	registerSprayClasses(scriptManager);

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
