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

bool RuntimePlugin::create(IEnvironment* environment)
{
	return true;
}

void RuntimePlugin::destroy(IEnvironment* environment)
{
}

Ref< IState > RuntimePlugin::createInitialState(IEnvironment* environment)
{
	Guid startupGuid(environment->getSettings()->getProperty< std::wstring >(L"Amalgam.Startup"));

	Ref< StageData > stageData = environment->getDatabase()->getObjectReadOnly< StageData >(startupGuid);
	if (!stageData)
	{
		log::error << L"Unable to read startup stage; startup failed" << Endl;
		return nullptr;
	}

	Ref< Stage > stage = stageData->createInstance(environment, nullptr);
	if (!stage)
	{
		log::error << L"Unable to create startup stage; startup failed" << Endl;
		return nullptr;
	}

	// Prepare all initial layers; this will cause pending resources and systems
	// to be created before first frame.
	const UpdateInfo info;
	for (const auto layer : stage->getLayers())
		layer->prepare(info);

	return new StageState(environment, stage);
}

	}
}
