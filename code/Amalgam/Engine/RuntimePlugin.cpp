#include "Amalgam/IEnvironment.h"
#include "Amalgam/Engine/GameEntityFactory.h"
#include "Amalgam/Engine/GameEntityRenderer.h"
#include "Amalgam/Engine/Layer.h"
#include "Amalgam/Engine/RuntimePlugin.h"
#include "Amalgam/Engine/Stage.h"
#include "Amalgam/Engine/StageData.h"
#include "Amalgam/Engine/StageState.h"
#include "Amalgam/Engine/Classes/AiClasses.h"
#include "Amalgam/Engine/Classes/AmalgamClasses.h"
#include "Amalgam/Engine/Classes/AnimationClasses.h"
#include "Amalgam/Engine/Classes/CoreClasses.h"
#include "Amalgam/Engine/Classes/DatabaseClasses.h"
#include "Amalgam/Engine/Classes/DrawingClasses.h"
#include "Amalgam/Engine/Classes/FlashClasses.h"
#include "Amalgam/Engine/Classes/I18NClasses.h"
#include "Amalgam/Engine/Classes/InputClasses.h"
#include "Amalgam/Engine/Classes/MeshClasses.h"
#include "Amalgam/Engine/Classes/NetClasses.h"
#include "Amalgam/Engine/Classes/OnlineClasses.h"
#include "Amalgam/Engine/Classes/PhysicsClasses.h"
#include "Amalgam/Engine/Classes/RenderClasses.h"
#include "Amalgam/Engine/Classes/ResourceClasses.h"
#include "Amalgam/Engine/Classes/SoundClasses.h"
#include "Amalgam/Engine/Classes/SprayClasses.h"
#include "Amalgam/Engine/Classes/TerrainClasses.h"
#include "Amalgam/Engine/Classes/WorldClasses.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"

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
	// Register script classes.
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

	// Add GameEntity to world renderer.
	environment->getWorld()->addEntityFactory(new GameEntityFactory(
		environment->getWorld()->getEntityEventManager()
	));
	environment->getWorld()->addEntityRenderer(new GameEntityRenderer());

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

	// Prepare all initial layers; this will cause pending resources and systems
	// to be created before first frame.
	const RefArray< Layer >& layers = stage->getLayers();
	for (RefArray< Layer >::const_iterator i = layers.begin(); i != layers.end(); ++i)
		(*i)->prepare();

	return new StageState(environment, stage);
}

	}
}
