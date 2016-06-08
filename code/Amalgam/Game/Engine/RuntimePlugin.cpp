#include "Amalgam/Game/IEnvironment.h"
#include "Amalgam/Game/UpdateInfo.h"
#include "Amalgam/Game/Engine/Layer.h"
#include "Amalgam/Game/Engine/RuntimePlugin.h"
#include "Amalgam/Game/Engine/Stage.h"
#include "Amalgam/Game/Engine/StageData.h"
#include "Amalgam/Game/Engine/StageState.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.RuntimePlugin", 0, RuntimePlugin, IRuntimePlugin)

bool RuntimePlugin::getDependencies(TypeInfoSet& outDependencies) const
{
	return true;
}

bool RuntimePlugin::startup(IEnvironment* environment)
{
	return true;
}

bool RuntimePlugin::shutdown(IEnvironment* environment)
{
	return true;
}

Ref< IState > RuntimePlugin::createInitialState(IEnvironment* environment) const
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
	const UpdateInfo info;
	const RefArray< Layer >& layers = stage->getLayers();
	for (RefArray< Layer >::const_iterator i = layers.begin(); i != layers.end(); ++i)
		(*i)->prepare(info);

	return new StageState(environment, stage);
}

	}
}
